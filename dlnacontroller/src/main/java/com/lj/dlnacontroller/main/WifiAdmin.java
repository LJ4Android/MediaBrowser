package com.lj.dlnacontroller.main;

import android.Manifest;
import android.annotation.TargetApi;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.provider.Settings;
import android.support.annotation.NonNull;
import android.widget.Toast;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.List;

/**
 * Created by wh on 2017/12/7.
 */

public class WifiAdmin {

    private final String TAG = "WifiAdmin";
    private final int REQUEST_CODE_WRITE_SETTINGS = 1;
    private final int REQUEST_ACCESS_FINE_LOCATION = 2;
    private final String WIFI_AP_STATE_CHANGED_ACTION = "android.net.wifi.WIFI_AP_STATE_CHANGED";
    private final String EXTRA_WIFI_AP_STATE = "wifi_state";
    private final int WIFI_AP_STATE_DISABLING = 10;
    private final int WIFI_AP_STATE_DISABLED = 11;
    private final int WIFI_AP_STATE_ENABLING = 12;
    private final int WIFI_AP_STATE_ENABLED = 13;
    private final int WIFI_AP_STATE_FAILED = 14;
    private WifiManager mWifiManager;
    private WifiManager.WifiLock wifiLock;
    private final String SSID = "Test";
    private final String PASSWD = "123456";
    private Context mContext;

    public WifiAdmin(Context context) {
        this.mContext = context;
        if (mWifiManager == null)
            mWifiManager = (WifiManager) context.getApplicationContext().getSystemService(Context.WIFI_SERVICE);
    }

    public void createGroup() {
        closeWifi();
        if (mContext != null) {
            registerWifiApReceiver();
            openWifiAp();
        }
    }

    public void connectGroup() {
        closeWifiAp();
        if (openWifi()) {
            registerScanWifiReceiver();
            scanWifi();
        }
    }

    @TargetApi(Build.VERSION_CODES.M)
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == REQUEST_CODE_WRITE_SETTINGS) {
            //解决6.0系统设置图标显示异常（默认打开）
            if (Settings.System.canWrite(mContext)) {
                openWifiAp();
            } else {
                if (mContext != null)
                    Toast.makeText(mContext, "获取权限失败!", Toast.LENGTH_LONG).show();
            }
        }
    }

    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        if (requestCode == REQUEST_ACCESS_FINE_LOCATION) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                searchGroup();
            } else {
                if (mContext != null)
                    Toast.makeText(mContext, "获取权限失败!", Toast.LENGTH_LONG).show();
            }
        }
    }

    private boolean openWifi() {
        if (!mWifiManager.isWifiEnabled()) {
            if (!mWifiManager.setWifiEnabled(true)) {
                Toast.makeText(mContext, "打开WIFI失败", Toast.LENGTH_LONG).show();
                return false;
            }
        }
        if (mWifiManager.isWifiEnabled()) {
            if(wifiLock == null) {
                wifiLock = mWifiManager.createWifiLock(WifiManager.WIFI_MODE_FULL_HIGH_PERF, "Test");
                if (wifiLock != null) wifiLock.acquire();
            }
        }
        return true;
    }

    private void closeWifi() {
        if (mWifiManager != null && mWifiManager.isWifiEnabled())
            mWifiManager.setWifiEnabled(false);
        if (wifiLock != null && wifiLock.isHeld())
            wifiLock.release();
    }

    private void openWifiAp() {
        if (mWifiManager == null) return;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (mContext != null && Settings.System.canWrite(mContext)) {
                setWifiApEnabled(createGroupConfiguration(SSID), true);
            } else {
                requestWriteSettings();
            }
        } else {
            setWifiApEnabled(createGroupConfiguration(SSID), true);
        }
    }

    private void closeWifiAp() {
        if (mWifiManager == null) return;
        if (isWifiApEnabled())
            setWifiApEnabled(getWifiApConfiguration(), false);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (mContext != null && Settings.System.canWrite(mContext)) {
                setWifiApEnabled(createGroupConfiguration(SSID), true);
            } else {
                requestWriteSettings();
            }
        } else {
            setWifiApEnabled(createGroupConfiguration(SSID), true);
        }
    }

    private void scanWifi() {
        if (mWifiManager != null) mWifiManager.startScan();
    }

    private void connectWifiAp() {
        if (mWifiManager == null) return;
        int wcgID = mWifiManager.addNetwork(createConnectConfiguration(SSID));
        boolean b=mWifiManager.enableNetwork(wcgID, true);
        if (mContext != null && b)
            Toast.makeText(mContext, "连接到群主", Toast.LENGTH_LONG).show();
    }

    private void setWifiApEnabled(WifiConfiguration wifiConfig, boolean enabled) {
        try {
            Method method = mWifiManager.getClass().getMethod("setWifiApEnabled", WifiConfiguration.class, boolean.class);
            method.invoke(mWifiManager, wifiConfig, enabled);
        } catch (NoSuchMethodException e) {
            LogHelper.logStackTrace(TAG, e);
        } catch (InvocationTargetException e) {
            LogHelper.logStackTrace(TAG, e);
        } catch (IllegalAccessException e) {
            LogHelper.logStackTrace(TAG, e);
        }
    }

    public boolean isWifiApEnabled() {
        try {
            Method method = mWifiManager.getClass().getMethod("isWifiApEnabled");
            return (boolean) method.invoke(mWifiManager);
        } catch (NoSuchMethodException e) {
            LogHelper.logStackTrace(TAG, e);
        } catch (InvocationTargetException e) {
            LogHelper.logStackTrace(TAG, e);
        } catch (IllegalAccessException e) {
            LogHelper.logStackTrace(TAG, e);
        }
        return false;
    }

    public WifiConfiguration getWifiApConfiguration() {
        try {
            Method method = mWifiManager.getClass().getMethod("getWifiApConfiguration");
            return (WifiConfiguration) method.invoke(mWifiManager);
        } catch (NoSuchMethodException e) {
            LogHelper.logStackTrace(TAG, e);
        } catch (InvocationTargetException e) {
            LogHelper.logStackTrace(TAG, e);
        } catch (IllegalAccessException e) {
            LogHelper.logStackTrace(TAG, e);
        }
        return null;
    }

    public WifiConfiguration createGroupConfiguration(String ssid) {
        WifiConfiguration config = new WifiConfiguration();
        config.SSID = ssid;
        config.hiddenSSID = true;
        config.allowedAuthAlgorithms
                .set(WifiConfiguration.AuthAlgorithm.OPEN);//开放系统认证
        config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);
        config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
        config.allowedPairwiseCiphers
                .set(WifiConfiguration.PairwiseCipher.TKIP);
        config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
        config.allowedPairwiseCiphers
                .set(WifiConfiguration.PairwiseCipher.CCMP);
        config.status = WifiConfiguration.Status.ENABLED;

        return config;
    }

    public WifiConfiguration createConnectConfiguration(String SSID) {
        WifiConfiguration configuration = new WifiConfiguration();
        configuration.allowedAuthAlgorithms.clear();
        configuration.allowedGroupCiphers.clear();
        configuration.allowedKeyManagement.clear();
        configuration.allowedPairwiseCiphers.clear();
        configuration.allowedProtocols.clear();
        configuration.SSID = "\"" + SSID + "\"";

        WifiConfiguration tempConfig = this.isExsits(SSID);
        if (tempConfig != null) {
            mWifiManager.removeNetwork(tempConfig.networkId);
        }

        configuration.hiddenSSID = true;
        configuration.allowedAuthAlgorithms.set(WifiConfiguration.AuthAlgorithm.OPEN);
        configuration.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);
        configuration.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
        configuration.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
        configuration.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);
        configuration.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.TKIP);
        configuration.status = WifiConfiguration.Status.ENABLED;
        return configuration;
    }

    private WifiConfiguration isExsits(String SSID) {
        List<WifiConfiguration> existingConfigs = mWifiManager.getConfiguredNetworks();
        for (WifiConfiguration existingConfig :
                existingConfigs) {
            if (existingConfig.SSID.equals("\"" + SSID + "\"")) {
                return existingConfig;
            }
        }
        return null;
    }

    @TargetApi(Build.VERSION_CODES.M)
    private void requestWriteSettings() {
        Intent intent = new Intent(Settings.ACTION_MANAGE_WRITE_SETTINGS);
        intent.setData(Uri.parse("package:" + mContext.getPackageName()));
        ((Activity) mContext).startActivityForResult(intent, REQUEST_CODE_WRITE_SETTINGS);
    }

    public void release(){
        unRegisterScanWifiReceiver();
        unRegisterWifiApReceiver();
    }

    private boolean mScanRegisted = false;

    private void registerScanWifiReceiver() {
        if (mContext == null) return;
        if (!mScanRegisted) {
            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION);
            mContext.registerReceiver(scanWifiReceiver, intentFilter);
            mScanRegisted = true;
        }
    }

    private void unRegisterScanWifiReceiver() {
        if (mScanRegisted && mContext != null) {
            mContext.unregisterReceiver(scanWifiReceiver);
            mScanRegisted = false;
        }
    }

    private boolean mWifiApRegisted = false;

    private void registerWifiApReceiver() {
        if (mContext != null && !mWifiApRegisted) {
            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(WIFI_AP_STATE_CHANGED_ACTION);
            mContext.registerReceiver(wifiApReceiver, intentFilter);
            mWifiApRegisted = true;
        }
    }

    private void unRegisterWifiApReceiver() {
        if (mContext != null && mWifiApRegisted) {
            mContext.unregisterReceiver(wifiApReceiver);
            mWifiApRegisted = false;
        }
    }

    private BroadcastReceiver wifiApReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (mContext == null) return;
            String action = intent.getAction();
            if (WIFI_AP_STATE_CHANGED_ACTION.equals(action)) {
                //便携式热点的状态为：10---正在关闭；11---已关闭；12---正在开启；13---已开启
                int state = intent.getIntExtra(EXTRA_WIFI_AP_STATE, 0);
                switch (state) {
                    case WIFI_AP_STATE_DISABLING:
                        Toast.makeText(mContext, "正在关闭", Toast.LENGTH_LONG).show();
                        break;
                    case WIFI_AP_STATE_DISABLED:
                        Toast.makeText(mContext, "已关闭", Toast.LENGTH_LONG).show();
                        break;
                    case WIFI_AP_STATE_ENABLED:
                        Toast.makeText(mContext, "已开启", Toast.LENGTH_LONG).show();
                        break;
                    case WIFI_AP_STATE_ENABLING:
                        Toast.makeText(mContext, "正在开启", Toast.LENGTH_LONG).show();
                        break;
                    case WIFI_AP_STATE_FAILED:
                        Toast.makeText(mContext, "创建群主失败!", Toast.LENGTH_LONG).show();
                        break;
                }
            }
        }
    };

    private BroadcastReceiver scanWifiReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                int checkPermission = mContext.checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION);
                LogHelper.logD(TAG, "checkPermission=" + checkPermission);
                if (checkPermission != PackageManager.PERMISSION_GRANTED) {
                    LogHelper.logD(TAG, "checkSelfPermission:" + false);
                    ((Activity) mContext).requestPermissions(new String[]{Manifest.permission.ACCESS_FINE_LOCATION}, REQUEST_ACCESS_FINE_LOCATION);
                } else {
                    LogHelper.logD(TAG, "checkSelfPermission:" + true);
                    searchGroup();
                }
            } else {
                searchGroup();
            }
        }
    };

    private void searchGroup() {
        List<ScanResult> results = getScanResults();
        if (results == null) {
            showSearchFail();
            return;
        }
        for (ScanResult result : results) {
            LogHelper.logD(TAG,result.SSID);
            if (SSID.equals(result.SSID)) {
                connectWifiAp();
                return;
            }
        }
        showSearchFail();
    }

    private void showSearchFail() {
        if (mContext == null) return;
        Toast.makeText(mContext, "搜索群主失败!", Toast.LENGTH_LONG).show();
    }

    private List<ScanResult> getScanResults() {
        try {
            if (mWifiManager != null)
                return mWifiManager.getScanResults();
        } catch (SecurityException e) {
            LogHelper.logStackTrace(TAG, e);
        }
        return null;
    }
}
