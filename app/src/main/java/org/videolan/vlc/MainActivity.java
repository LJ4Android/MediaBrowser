package org.videolan.vlc;

import android.os.Handler;
import android.os.Message;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.view.WindowManager;
import android.widget.ProgressBar;
import android.widget.TextView;

import org.videolan.vlc.gui.video.VideoGridFragment;
import org.videolan.vlc.gui.video.VideoListAdapter;
import org.videolan.vlc.interfaces.ISortable;
import org.videolan.vlc.media.MediaLibrary;
import org.videolan.vlc.util.WeakHandler;

public class MainActivity extends AppCompatActivity {

    private Handler mHandler = new MainActivityHandler(this);
    private static final int ACTIVITY_SHOW_INFOLAYOUT = 2;
    private static final int ACTIVITY_SHOW_PROGRESSBAR = 3;
    private static final int ACTIVITY_HIDE_PROGRESSBAR = 4;
    private static final int ACTIVITY_SHOW_TEXTINFO = 5;

    private View mInfoLayout;
    private ProgressBar mInfoProgress;
    private TextView mInfoText;

    private VideoGridFragment mFragment;

    MediaLibrary mMediaLibrary;
    private boolean mScanNeeded;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mMediaLibrary = MediaLibrary.getInstance();
        if (!mMediaLibrary.isWorking() && mMediaLibrary.getMediaItems().isEmpty()) {
                mMediaLibrary.scanMediaItems();
        }

        setContentView(R.layout.activity_main);
        mInfoLayout=findViewById(R.id.info_layout);
        mInfoProgress= (ProgressBar) findViewById(R.id.info_progress);
        mInfoText= (TextView) findViewById(R.id.info_text);
        mFragment=new VideoGridFragment();
        FragmentTransaction ft = getSupportFragmentManager().beginTransaction();
        ft.replace(R.id.fragment_contain, mFragment);
        ft.commit();
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mScanNeeded)
            mMediaLibrary.scanMediaItems();
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (getChangingConfigurations() == 0) {
            /* Check for an ongoing scan that needs to be resumed during onResume */
            mScanNeeded = mMediaLibrary.isWorking();
            /* Stop scanning for files */
            mMediaLibrary.stop();
        }
    }

    public void sortTitle(View v){
        int sortBy = VideoListAdapter.SORT_BY_TITLE;
        sortBy(sortBy);
    }

    public void sortLength(View v){
        int sortBy = VideoListAdapter.SORT_BY_LENGTH;
        sortBy(sortBy);
    }

    public void sortTime(View v){
        int sortBy = VideoListAdapter.SORT_BY_DATE;
        sortBy(sortBy);
    }


    private void sortBy(int sortBy) {
        if(mFragment!=null)
            mFragment.sortBy(sortBy);
    }

    public void sendTextInfo(String info, int progress, int max) {
        mHandler.obtainMessage(ACTIVITY_SHOW_TEXTINFO, max, progress, info).sendToTarget();
    }

    public void clearTextInfo() {
        mHandler.obtainMessage(ACTIVITY_SHOW_TEXTINFO, 0, 100, null).sendToTarget();
    }

    public void hideProgressBar() {
        mHandler.obtainMessage(ACTIVITY_HIDE_PROGRESSBAR).sendToTarget();
    }

    public void showProgressBar() {
        mHandler.obtainMessage(ACTIVITY_SHOW_PROGRESSBAR).sendToTarget();
    }

    private static class MainActivityHandler extends WeakHandler<MainActivity> {
        public MainActivityHandler(MainActivity owner) {
            super(owner);
        }

        @Override
        public void handleMessage(Message msg) {
            MainActivity ma = getOwner();
            if(ma == null) return;

            switch (msg.what) {
                case ACTIVITY_SHOW_INFOLAYOUT:
                    ma.mInfoLayout.setVisibility(View.VISIBLE);
                    break;
                case ACTIVITY_SHOW_PROGRESSBAR:
                    ma.mInfoProgress.setVisibility(View.VISIBLE);
                    ma.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                    break;
                case ACTIVITY_HIDE_PROGRESSBAR:
                    ma.mInfoProgress.setVisibility(View.GONE);
                    ma.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                    break;
                case ACTIVITY_SHOW_TEXTINFO:
                    String info = (String) msg.obj;
                    int max = msg.arg1;
                    int progress = msg.arg2;
                    ma.mInfoText.setText(info);
                    ma.mInfoProgress.setMax(max);
                    ma.mInfoProgress.setProgress(progress);

                    if (info == null) {
                    /* Cancel any upcoming visibility change */
                        removeMessages(ACTIVITY_SHOW_INFOLAYOUT);
                        ma.mInfoLayout.setVisibility(View.GONE);
                    } else {
                    /* Slightly delay the appearance of the progress bar to avoid unnecessary flickering */
                        if (!hasMessages(ACTIVITY_SHOW_INFOLAYOUT))
                            sendEmptyMessageDelayed(ACTIVITY_SHOW_INFOLAYOUT, 300);
                    }
                    break;
            }
        }
    }
}
