package com.example.smartdoor;

import androidx.appcompat.app.AppCompatActivity;

import android.Manifest;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import androidx.core.app.ActivityCompat;
import androidx.core.app.NotificationCompat;
import androidx.core.content.ContextCompat;

import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;

import java.io.IOException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "SmartDoor";
    private static final String URL = "http://192.168.169.1/motion.html";
    private static final String RFID_STATUS_URL = "http://192.168.169.1/doorstatus.html";
    private static final String RFID_TAGS_URL = "http://192.168.169.1/rfidtags.html";
    private static final String CLEAR_MOTION_LOGS_URL = "http://192.168.169.1/clearmotion.html";
    private static final String OPEN_DOOR_URL = "http://192.168.169.1/opendoor.html";

    private TextView statusTextView;
    private TextView rfidStatusTextView;
    private TextView rfidTagsTextView;
    private ImageView rfidStatusIcon;
    private Button clearMotionLogsButton;
    private Button exitButton;
    private Button openDoorButton;

    private boolean isRunning = true;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Request notification permission
        requestNotificationPermission();

        // Initialize UI components
        statusTextView = findViewById(R.id.statusTextView);
        rfidStatusTextView = findViewById(R.id.rfidStatusTextView);
        rfidTagsTextView = findViewById(R.id.rfidTagsTextView);
        rfidStatusIcon = findViewById(R.id.rfidStatusIcon);
        clearMotionLogsButton = findViewById(R.id.clearMotionLogsButton);
        exitButton = findViewById(R.id.exitButton);
        openDoorButton = findViewById(R.id.openDoorButton);

        // Set up button listeners
        clearMotionLogsButton.setOnClickListener(this::clearMotionLogs);
        exitButton.setOnClickListener(v -> finish());
        openDoorButton.setOnClickListener(this::openDoor);

        // Start monitoring
        startMonitoring();
        startMonitoringRfidStatus();
        startMonitoringRfidTags();
    }

    private void requestNotificationPermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.POST_NOTIFICATIONS)
                    != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.POST_NOTIFICATIONS}, 1);
            }
        }
    }

    private void startMonitoring() {
        new Thread(() -> {
            Log.d(TAG, "Starting motion monitoring");

            OkHttpClient client = new OkHttpClient.Builder()
                    .connectTimeout(10, java.util.concurrent.TimeUnit.SECONDS)
                    .readTimeout(10, java.util.concurrent.TimeUnit.SECONDS)
                    .build();

            while (isRunning) {
                try {
                    Request request = new Request.Builder().url(URL).build();

                    try (Response response = client.newCall(request).execute()) {
                        if (response.isSuccessful()) {
                            String responseBody = response.body().string();
                            Log.d(TAG, "Response received: " + responseBody);

                            String motionStatus = parseHtmlForStatus(responseBody);

                            runOnUiThread(() -> {
                                statusTextView.setText(getString(R.string.status_prefix) + motionStatus);

                                if ("Motion detected".equals(motionStatus)) {
                                    statusTextView.setTextColor(Color.RED);
                                    showNotification(getString(R.string.motion_alert), motionStatus);
                                } else {
                                    statusTextView.setTextColor(getResources().getColor(R.color.primary_text));
                                }
                            });
                        } else {
                            Log.e(TAG, "HTTP Error. Code: " + response.code());
                            runOnUiThread(() -> statusTextView.setText(getString(R.string.error_code, response.code())));
                        }
                    }

                    Thread.sleep(5000);
                } catch (IOException | InterruptedException e) {
                    Log.e(TAG, "Error during monitoring: " + e.getMessage(), e);
                    runOnUiThread(() -> Toast.makeText(this, getString(R.string.error_fetching_status), Toast.LENGTH_SHORT).show());
                }
            }
        }).start();
    }

    private void startMonitoringRfidStatus() {
        new Thread(() -> {
            Log.d(TAG, "Starting RFID status monitoring");

            OkHttpClient client = new OkHttpClient.Builder()
                    .connectTimeout(10, java.util.concurrent.TimeUnit.SECONDS)
                    .readTimeout(10, java.util.concurrent.TimeUnit.SECONDS)
                    .build();

            while (isRunning) {
                try {
                    Request request = new Request.Builder().url(RFID_STATUS_URL).build();

                    try (Response response = client.newCall(request).execute()) {
                        if (response.isSuccessful()) {
                            String responseBody = response.body().string();
                            Log.d(TAG, "RFID Status Response received: " + responseBody);

                            String rfidStatusMessage = parseRfidStatus(responseBody);

                            runOnUiThread(() -> {
                                rfidStatusTextView.setText(rfidStatusMessage);
                                if (rfidStatusMessage.contains("Door Open")) {
                                    rfidStatusTextView.setTextColor(Color.RED);
                                    rfidStatusIcon.setImageResource(R.drawable.dooropen);
                                } else {
                                    rfidStatusTextView.setTextColor(getResources().getColor(R.color.primary_text));
                                    rfidStatusIcon.setImageResource(R.drawable.doorclose);
                                }
                            });

                            if (rfidStatusMessage.contains("Door Open")) {
                                showNotification(getString(R.string.rfid_alert), rfidStatusMessage);
                            }
                        } else {
                            Log.e(TAG, "HTTP Error for RFID Status. Code: " + response.code());
                            runOnUiThread(() -> rfidStatusTextView.setText(getString(R.string.error_code, response.code())));
                        }
                    }

                    Thread.sleep(5000);
                } catch (IOException | InterruptedException e) {
                    Log.e(TAG, "Error during RFID status monitoring: " + e.getMessage(), e);
                    runOnUiThread(() -> Toast.makeText(this, getString(R.string.error_fetching_status), Toast.LENGTH_SHORT).show());
                }
            }
        }).start();
    }

    private void startMonitoringRfidTags() {
        new Thread(() -> {
            Log.d(TAG, "Starting RFID tags monitoring");

            OkHttpClient client = new OkHttpClient.Builder()
                    .connectTimeout(10, java.util.concurrent.TimeUnit.SECONDS)
                    .readTimeout(10, java.util.concurrent.TimeUnit.SECONDS)
                    .build();

            while (isRunning) {
                try {
                    Request request = new Request.Builder().url(RFID_TAGS_URL).build();

                    try (Response response = client.newCall(request).execute()) {
                        if (response.isSuccessful()) {
                            String responseBody = response.body().string();
                            String userId = extractUserIdFromTags(responseBody);
                            runOnUiThread(() -> rfidTagsTextView.setText("Last Scanned User ID: " + userId));
                        }
                    }

                    Thread.sleep(5000);
                } catch (Exception e) {
                    Log.e(TAG, "Error during RFID tags monitoring", e);
                }
            }
        }).start();
    }

    private void clearMotionLogs(View view) {
        new Thread(() -> {
            Log.d(TAG, "Clearing motion logs");

            OkHttpClient client = new OkHttpClient();

            try {
                Request request = new Request.Builder().url(CLEAR_MOTION_LOGS_URL).build();
                try (Response response = client.newCall(request).execute()) {
                    runOnUiThread(() -> Toast.makeText(this, "Motion logs cleared", Toast.LENGTH_SHORT).show());
                }
            } catch (Exception e) {
                Log.e(TAG, "Error clearing motion logs", e);
            }
        }).start();
    }

    private void openDoor(View view) {
        new Thread(() -> {
            Log.d(TAG, "Sending open door request");

            OkHttpClient client = new OkHttpClient();

            try {
                Request request = new Request.Builder().url(OPEN_DOOR_URL).build();
                try (Response response = client.newCall(request).execute()) {
                    if (response.isSuccessful()) {
                        runOnUiThread(() -> Toast.makeText(this, "Door opened successfully", Toast.LENGTH_SHORT).show());
                    } else {
                        runOnUiThread(() -> Toast.makeText(this, "Failed to open door", Toast.LENGTH_SHORT).show());
                    }
                }
            } catch (Exception e) {
                Log.e(TAG, "Error sending open door request", e);
                runOnUiThread(() -> Toast.makeText(this, "Error: Could not open door", Toast.LENGTH_SHORT).show());
            }
        }).start();
    }

    private String parseHtmlForStatus(String html) {
        if (html.contains("Motion detected")) {
            return "Motion detected";
        } else if (html.contains("No motion")) {
            return "No motion detected";
        } else {
            return "No motion detected";
        }
    }

    private String parseRfidStatus(String html) {
        if (html.contains("Door Open")) {
            String userId = extractUserId(html);
            return "Door Open";
        } else if (html.contains("Door Closed")) {
            return "Door remains closed";
        } else {
            return "Door remains closed";
        }
    }

    private String extractUserId(String html) {
        Pattern pattern = Pattern.compile("Verifying RFID Tag:\\s*(\\w+)");
        Matcher matcher = pattern.matcher(html);
        if (matcher.find()) {
            return matcher.group(1);
        }
        return "Unknown";
    }

    private String extractUserIdFromTags(String html) {
        Pattern pattern = Pattern.compile("Tag Owner:\\s*(\\w+)");
        Matcher matcher = pattern.matcher(html);
        if (matcher.find()) {
            return matcher.group(1);
        }
        return "Unknown";
    }

    private void showNotification(String title, String message) {
        NotificationManager notificationManager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(
                    "motion_channel",
                    "Motion and RFID Alerts",
                    NotificationManager.IMPORTANCE_HIGH
            );
            notificationManager.createNotificationChannel(channel);
        }

        NotificationCompat.Builder builder = new NotificationCompat.Builder(this, "motion_channel")
                .setSmallIcon(R.drawable.ic_notification)
                .setContentTitle(title)
                .setContentText(message)
                .setPriority(NotificationCompat.PRIORITY_HIGH);

        notificationManager.notify((int) System.currentTimeMillis(), builder.build());
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        isRunning = false;
    }
}
