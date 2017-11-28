package com.lj.videoplayer.main;

import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;

import com.lj.videoplayer.R;
import com.lj.videoplayer.api.io.haydar.filescanner.FileInfo;
import com.lj.videoplayer.api.io.haydar.filescanner.FileScanner;

import java.util.ArrayList;


public class MainActivity extends AppCompatActivity {
    public static final String TAG = "MainActivity";


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        System.out.println(Environment.getExternalStorageDirectory().getAbsolutePath());
    }

    public void fileScanner() {
        //FileScanner.getInstance(this).clear();
        FileScanner.getInstance(this).setType(".mp3").start(new FileScanner.ScannerListener() {
            @Override
            public void onScanBegin() {
                Log.d(TAG, "onScanBegin: ");
            }

            @Override
            public void onScanEnd() {
                //   Log.d(TAG, "onScanEnd: ");
                ArrayList<FileInfo> fileInfoArrayList = FileScanner.getInstance(MainActivity.this).getAllFiles();
                for (FileInfo fileInfo : fileInfoArrayList) {
                    Log.d(TAG, "fileScanner: " + fileInfo.getFilePath());
                }
            }

            @Override
            public void onScanning(String paramString, int progress) {
                Log.d(TAG, "onScanning: " + progress);
            }

            @Override
            public void onScanningFiles(FileInfo info, int type) {
                Log.d(TAG, "onScanningFiles: info=" + info.toString());
                //  Log.d(TAG, "onScanningFiles: type=" + type);
            }
        });

    }


}
