/*****************************************************************************
 * VideoListActivity.java
 *****************************************************************************
 * Copyright © 2011-2012 VLC authors and VideoLAN
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

package org.videolan.vlc.gui.video;

import android.Manifest;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.os.Bundle;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.support.annotation.NonNull;
import android.support.v4.app.FragmentActivity;
import android.support.v4.content.LocalBroadcastManager;
import android.support.v4.util.ArrayMap;
import android.support.v7.widget.RecyclerView;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import org.videolan.vlc.MainActivity;
import org.videolan.vlc.R;
import org.videolan.vlc.VLCApplication;
import org.videolan.vlc.gui.browser.MediaBrowserFragment;
import org.videolan.vlc.gui.view.AutoFitRecyclerView;
import org.videolan.vlc.gui.view.SwipeRefreshLayout;
import org.videolan.vlc.interfaces.ISortable;
import org.videolan.vlc.interfaces.IVideoBrowser;
import org.videolan.vlc.media.MediaDatabase;
import org.videolan.vlc.media.MediaGroup;
import org.videolan.vlc.media.MediaLibrary;
import org.videolan.vlc.media.MediaUtils;
import org.videolan.vlc.media.MediaWrapper;
import org.videolan.vlc.media.Thumbnailer;
import org.videolan.vlc.util.FileUtils;

import java.util.ArrayList;
import java.util.List;

import pub.devrel.easypermissions.EasyPermissions;

public class VideoGridFragment extends MediaBrowserFragment implements ISortable, IVideoBrowser, SwipeRefreshLayout.OnRefreshListener, EasyPermissions.PermissionCallbacks {

    public final static String TAG = "VLC/VideoListFragment";

    private static final int WRITE_EXTERNAL_STORAGE = 122;

    public final static String KEY_GROUP = "key_group";

    protected LinearLayout mLayoutFlipperLoading;
    protected AutoFitRecyclerView mGridView;
    protected TextView mTextViewNomedia;
    protected View mViewNomedia;
    protected String mGroup;

    private VideoListAdapter mVideoAdapter;
    private Thumbnailer mThumbnailer;
    private VideoGridAnimator mAnimator;
    private boolean animate;
    private ArrayMap<String, Long> times;

    private MainActivity mMainActivity;

    /* All subclasses of Fragment must include a public empty constructor. */
    public VideoGridFragment() {
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mVideoAdapter = new VideoListAdapter(this);

        if (savedInstanceState != null)
            setGroup(savedInstanceState.getString(KEY_GROUP));
        /* Load the thumbnailer */
        FragmentActivity activity = getActivity();
        if (activity != null)
            mThumbnailer = new Thumbnailer();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {

        View v = inflater.inflate(R.layout.video_grid, container, false);

        // init the information for the scan (1/2)
        mLayoutFlipperLoading = (LinearLayout) v.findViewById(R.id.layout_flipper_loading);
        mTextViewNomedia = (TextView) v.findViewById(R.id.textview_nomedia);
        mViewNomedia = v.findViewById(android.R.id.empty);
        mGridView = (AutoFitRecyclerView) v.findViewById(android.R.id.list);
        mSwipeRefreshLayout = (SwipeRefreshLayout) v.findViewById(R.id.swipeLayout);

        mSwipeRefreshLayout.setColorSchemeResources(R.color.orange700);
        mSwipeRefreshLayout.setOnRefreshListener(this);

        mGridView.addOnScrollListener(mScrollListener);
        mGridView.setAdapter(mVideoAdapter);
        Log.d(TAG, "onCreateView");
        return v;
    }

    RecyclerView.OnScrollListener mScrollListener = new RecyclerView.OnScrollListener() {
        @Override
        public void onScrollStateChanged(RecyclerView recyclerView, int newState) {
            super.onScrollStateChanged(recyclerView, newState);
        }

        @Override
        public void onScrolled(RecyclerView recyclerView, int dx, int dy) {
            int topRowVerticalPosition =
                    (recyclerView == null || recyclerView.getChildCount() == 0) ? 0 : recyclerView.getChildAt(0).getTop();
            mSwipeRefreshLayout.setEnabled(topRowVerticalPosition >= 0);
        }
    };

    @Override
    public void onViewCreated(View view, Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        // init the information for the scan (2/2)
        IntentFilter filter = new IntentFilter();
        filter.addAction(MediaUtils.ACTION_SCAN_START);
        filter.addAction(MediaUtils.ACTION_SCAN_STOP);
        LocalBroadcastManager.getInstance(getActivity()).registerReceiver(messageReceiverVideoListFragment, filter);
        if (mMediaLibrary.isWorking()) {
            MediaUtils.actionScanStart();
        }
        Log.d(TAG, "onViewCreated");
        mAnimator = new VideoGridAnimator(mGridView);
    }

    @Override
    public void onPause() {
        super.onPause();
        Log.d(TAG, "onPause");
        mMediaLibrary.setBrowser(null);
        mMediaLibrary.removeUpdateHandler(mHandler);

        /* Stop the thumbnailer */
        if (mThumbnailer != null)
            mThumbnailer.stop();
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.d(TAG, "onResume");
        if (getActivity() instanceof MainActivity)
            mMainActivity = (MainActivity) getActivity();
        mMediaLibrary.setBrowser(this);
        mMediaLibrary.addUpdateHandler(mHandler);
        final boolean refresh = mVideoAdapter.isEmpty() && !mMediaLibrary.isWorking();
        // We don't animate while medialib is scanning. Because gridview is being populated.
        // That would lead to graphical glitches
        animate = mGroup == null && refresh;
        Log.d(TAG, "onResume_refresh:" + refresh);
        if (refresh)
            updateList();
        else {
            mViewNomedia.setVisibility(mVideoAdapter.getItemCount() > 0 ? View.GONE : View.VISIBLE);
        }
        //Get & set times
        times = MediaDatabase.getInstance().getVideoTimes();


        if (EasyPermissions.hasPermissions(getContext(), Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
            // Have permission, do the thing!
            Toast.makeText(getActivity(), "HAS_WRITE_EXTERNAL_STORAGE ", Toast.LENGTH_LONG).show();

            mVideoAdapter.setTimes(times);
            updateViewMode();
            if (animate)
                mAnimator.animate();
        } else {
            // Request one permission
            EasyPermissions.requestPermissions(this, getString(R.string.rationale_write_external_storage),
                    WRITE_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        EasyPermissions.onRequestPermissionsResult(requestCode, permissions, grantResults, this);
        Log.e("tlh", "onRequestPermissionsResult:");
    }

    @Override
    public void onPermissionsGranted(int requestCode, List<String> perms) {
        Log.e("tlh", "onPermissionsGranted:" + requestCode + ":" + perms.size());
        scanMediaItems();

    }

    @Override
    public void onPermissionsDenied(int requestCode, List<String> perms) {

        Log.e("tlh", "onPermissionsDenied:" + requestCode + ":" + perms.size());
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putString(KEY_GROUP, mGroup);
        Log.d(TAG, "onSaveInstanceState");
    }

    @Override
    public void onDestroyView() {
        LocalBroadcastManager.getInstance(getActivity()).unregisterReceiver(messageReceiverVideoListFragment);
        super.onDestroyView();
        Log.d(TAG, "onDestroyView");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (mThumbnailer != null)
            mThumbnailer.clearJobs();
        mVideoAdapter.clear();
        Log.d(TAG, "onDestroy");
    }

    protected String getTitle() {
        if (mGroup == null)
            return getString(R.string.video);
        else
            return mGroup + "\u2026";
    }

    private void updateViewMode() {
        if (getView() == null || getActivity() == null) {
            Log.w(TAG, "Unable to setup the view");
            return;
        }
        Resources res = getResources();
        boolean listMode = res.getBoolean(R.bool.list_mode);
        listMode |= res.getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT &&
                PreferenceManager.getDefaultSharedPreferences(getActivity()).getBoolean("force_list_portrait", false);
        // Compute the left/right padding dynamically
        DisplayMetrics outMetrics = new DisplayMetrics();
        getActivity().getWindowManager().getDefaultDisplay().getMetrics(outMetrics);

        // Select between grid or list
        if (!listMode) {
            mGridView.setNumColumns(-1);
            int thumbnailWidth = res.getDimensionPixelSize(R.dimen.grid_card_thumb_width);
            mGridView.setColumnWidth(mGridView.getPerfectColumnWidth(thumbnailWidth, res.getDimensionPixelSize(R.dimen.default_margin)));
        } else {
            mGridView.setNumColumns(1);
        }
        mVideoAdapter.setListMode(listMode);
    }

    protected void playVideo(MediaWrapper media, boolean fromStart) {
        media.removeFlags(MediaWrapper.MEDIA_FORCE_AUDIO);
        //VideoPlayerActivity.start(getActivity(), media.getUri(), fromStart);
    }

    /**
     * Handle changes on the list
     */
    private Handler mHandler = new VideoListHandler(this);

    public void updateItem(MediaWrapper item) {
        if (item.getType() != MediaWrapper.TYPE_VIDEO)
            return;
        mVideoAdapter.update(item);
        mViewNomedia.setVisibility(mVideoAdapter.getItemCount() > 0 ? View.GONE : View.VISIBLE);
    }

    public void updateList() {
        if (!mSwipeRefreshLayout.isRefreshing())
            mSwipeRefreshLayout.setRefreshing(true);
        final List<MediaWrapper> itemList = mMediaLibrary.getVideoItems();

        if (itemList.size() > 0) {
            VLCApplication.runBackground(new Runnable() {
                @Override
                public void run() {
                    final ArrayList<MediaWrapper> displayList = new ArrayList<>();
                    final ArrayList<MediaWrapper> jobsList = new ArrayList<>();
                    if (mGroup != null || itemList.size() <= 10) {
                        for (MediaWrapper item : itemList) {
                            String title = item.getTitle().substring(item.getTitle().toLowerCase().startsWith("the") ? 4 : 0);
                            if (mGroup == null || title.toLowerCase().startsWith(mGroup.toLowerCase()))
                                displayList.add(item);
                            jobsList.add(item);
                        }
                    } else {
                        List<MediaGroup> groups = MediaGroup.group(itemList);
                        for (MediaGroup item : groups) {
                            displayList.add(item.getMedia());
                            for (MediaWrapper media : item.getAll())
                                jobsList.add(media);
                        }
                    }

                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            mVideoAdapter.clear();
                            Log.d(TAG, "updateList " + displayList.size());
                            mVideoAdapter.addAll(displayList);
                            if (mReadyToDisplay)
                                display();
                        }
                    });
                    if (mThumbnailer != null && !jobsList.isEmpty()) {
                        mThumbnailer.clearJobs();
                        mThumbnailer.start(VideoGridFragment.this);
                        for (MediaWrapper item : jobsList)
                            mThumbnailer.addJob(item);
                    }
                }
            });
        }
        stopRefresh();
    }

    @Override
    public void showProgressBar() {
        if (mMainActivity != null)
            mMainActivity.showProgressBar();
    }

    @Override
    public void hideProgressBar() {
        if (mMainActivity != null)
            mMainActivity.hideProgressBar();
    }

    @Override
    public void clearTextInfo() {
        if (mMainActivity != null)
            mMainActivity.clearTextInfo();
    }

    @Override
    public void sendTextInfo(String info, int progress, int max) {
        if (mMainActivity != null)
            mMainActivity.sendTextInfo(info, progress, max);
    }

    @Override
    public void sortBy(int sortby) {
        mVideoAdapter.sortBy(sortby);
    }

    @Override
    public int sortDirection(int sortby) {
        return mVideoAdapter.sortDirection(sortby);
    }

    public void setItemToUpdate(MediaWrapper item) {
        if (mVideoAdapter.contains(item))
            mHandler.sendMessage(mHandler.obtainMessage(MediaLibrary.UPDATE_ITEM, item));
        else // Update group item when its first element is updated
            for (int i = 0; i < mVideoAdapter.getItemCount(); ++i) {
                if (mVideoAdapter.getItem(i) instanceof MediaGroup &&
                        ((MediaGroup) mVideoAdapter.getItem(i)).getFirstMedia().equals(item)) {
                    final int position = i;
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            mVideoAdapter.notifyItemChanged(position);
                        }
                    });
                    return;
                }
            }
    }

    public void setGroup(String prefix) {
        mGroup = prefix;
    }

    private final BroadcastReceiver messageReceiverVideoListFragment = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            if (action.equalsIgnoreCase(MediaUtils.ACTION_SCAN_START)) {
                mLayoutFlipperLoading.setVisibility(View.VISIBLE);
            } else if (action.equalsIgnoreCase(MediaUtils.ACTION_SCAN_STOP)) {
                mLayoutFlipperLoading.setVisibility(View.INVISIBLE);
            }
        }
    };

    public void stopRefresh() {
        mSwipeRefreshLayout.setRefreshing(false);
    }

    @Override
    public void onRefresh() {
        scanMediaItems();
    }

    private void scanMediaItems() {
        if (getActivity() != null && !MediaLibrary.getInstance().isWorking())
            MediaLibrary.getInstance().scanMediaItems(true);
    }

    @Override
    public void display() {
        if (getActivity() != null)
            getActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mVideoAdapter.notifyDataSetChanged();
                    mViewNomedia.setVisibility(mVideoAdapter.getItemCount() > 0 ? View.GONE : View.VISIBLE);
                    mReadyToDisplay = true;
                    mGridView.requestFocus();
                }
            });
    }

    public void clear() {
        mVideoAdapter.clear();
    }

    public void deleteMedia(final MediaWrapper media) {
        VLCApplication.runBackground(new Runnable() {
            @Override
            public void run() {
                FileUtils.deleteFile(media.getUri().getPath());
                MediaDatabase.getInstance().removeMedia(media.getUri());
            }
        });
        mMediaLibrary.getMediaItems().remove(media);
    }


}
