package org.videolan.vlc.media;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.provider.MediaStore;
import android.support.v4.content.LocalBroadcastManager;
import android.text.TextUtils;
import android.util.Log;

import org.videolan.libvlc.util.AndroidUtil;
import org.videolan.vlc.R;
import org.videolan.vlc.VLCApplication;
import org.videolan.vlc.util.FileUtils;
import org.videolan.vlc.util.Strings;

import java.util.ArrayList;
import java.util.List;

public class MediaUtils {
    private static final String TAG="VLC/MediaUtils";

    public static final String ACTION_SCAN_START = Strings.buildPkgString("gui.ScanStart");
    public static final String ACTION_SCAN_STOP = Strings.buildPkgString("gui.ScanStop");


    public static void actionScanStart() {
        Intent intent = new Intent();
        intent.setAction(ACTION_SCAN_START);
        LocalBroadcastManager.getInstance(VLCApplication.getAppContext()).sendBroadcast(intent);
        Log.d(TAG,"actionScanStart");
    }

    public static void actionScanStop() {
        Intent intent = new Intent();
        intent.setAction(ACTION_SCAN_STOP);
        LocalBroadcastManager.getInstance(VLCApplication.getAppContext()).sendBroadcast(intent);
    }


    public static String getMediaArtist(Context ctx, MediaWrapper media) {
        final String artist = media.getArtist();
        return artist != null ? artist : getMediaString(ctx, R.string.unknown_artist);
    }

    public static String getMediaReferenceArtist(Context ctx, MediaWrapper media) {
        final String artist = media.getReferenceArtist();
        return artist != null ? artist : getMediaString(ctx, R.string.unknown_artist);
    }

    public static String getMediaAlbumArtist(Context ctx, MediaWrapper media) {
        final String albumArtist = media.getAlbumArtist();
        return albumArtist != null ? albumArtist : getMediaString(ctx, R.string.unknown_artist);
    }

    public static String getMediaAlbum(Context ctx, MediaWrapper media) {
        final String album = media.getAlbum();
        return album != null ? album : getMediaString(ctx, R.string.unknown_album);

    }

    public static String getMediaGenre(Context ctx, MediaWrapper media) {
        final String genre = media.getGenre();
        return genre != null ? genre : getMediaString(ctx, R.string.unknown_genre);
    }

    public static String getMediaSubtitle(MediaWrapper media) {
        String subtitle = media.getNowPlaying() != null
                ? media.getNowPlaying()
                : media.getArtist();
        if (media.getLength() > 0L) {
            if (TextUtils.isEmpty(subtitle))
                subtitle = Strings.millisToString(media.getLength());
            else
                subtitle = subtitle + "  -  " +  Strings.millisToString(media.getLength());
        }
        return subtitle;
    }

    public static String getMediaTitle(MediaWrapper mediaWrapper){
        String title = mediaWrapper.getTitle();
        if (title == null)
            title = FileUtils.getFileNameFromPath(mediaWrapper.getLocation());
        return title;
    }

    public static Uri getContentMediaUri(Uri data) {
        Uri uri = null;
        try {
            Cursor cursor = VLCApplication.getAppContext().getContentResolver().query(data,
                    new String[]{ MediaStore.Video.Media.DATA }, null, null, null);
            if (cursor != null) {
                int column_index = cursor.getColumnIndexOrThrow(MediaStore.Video.Media.DATA);
                if (cursor.moveToFirst())
                    uri = AndroidUtil.PathToUri(cursor.getString(column_index));
                cursor.close();
            } else // other content-based URI (probably file pickers)
                uri = data;
        } catch (Exception e) {
            uri = data;
            if (uri.getScheme() == null)
                uri = AndroidUtil.PathToUri(uri.getPath());
        }
        return uri != null ? uri : data;
    }
    private static String getMediaString(Context ctx, int id) {
        if (ctx != null)
            return ctx.getResources().getString(id);
        else {
            switch (id) {
                case R.string.unknown_artist:
                    return "Unknown Artist";
                case R.string.unknown_album:
                    return "Unknown Album";
                case R.string.unknown_genre:
                    return "Unknown Genre";
                default:
                    return "";
            }
        }
    }
}
