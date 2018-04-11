package com.lj.dlnacontroller.main;

import android.util.Log;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.io.Writer;

/**
 * Created by wh on 2017/12/7.
 */

public final class LogHelper {

    public static void logStackTrace(String tag, Throwable e) {
        Writer writer = new StringWriter();
        PrintWriter printWriter = new PrintWriter(writer);
        Throwable cause = e.getCause();
        while (cause != null) {
            cause.printStackTrace(printWriter);
            cause = cause.getCause();
        }
        printWriter.close();
        String result = writer.toString();
        Log.d(tag, result);
    }

    public static void logD(String tag,String msg){
        Log.d(tag,msg);
    }
}
