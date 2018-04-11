package com.lj.dlnacontroller.api;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * Created by wh on 2017/12/16.
 */

public class MediaObject implements Parcelable {
    private String m_ObjectID;
    private String m_ParentID;
    private String m_Type;
    private String m_Title;
    private String m_Uri;
    private String m_Didl;
    private long   m_Size;

    public MediaObject() {}

    public String getObjectID() {
        return m_ObjectID;
    }

    public void setObjectID(String m_ObjectID) {
        this.m_ObjectID = m_ObjectID;
    }

    public String getParentID() {
        return m_ParentID;
    }

    public void setParentID(String m_ParentID) {
        this.m_ParentID = m_ParentID;
    }

    public String getType() {
        return m_Type;
    }

    public void setType(String m_Type) {
        this.m_Type = m_Type;
    }

    public String getTitle() {
        return m_Title;
    }

    public void setTitle(String m_Title) {
        this.m_Title = m_Title;
    }

    public String getUri() {
        return m_Uri;
    }

    public void setUri(String m_Uri) {
        this.m_Uri = m_Uri;
    }

    public String getDidl() {
        return m_Didl;
    }

    public void setDidl(String m_Didl) {
        this.m_Didl = m_Didl;
    }

    public long getSize() {
        return m_Size;
    }

    public void setSize(long m_Size) {
        this.m_Size = m_Size;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(this.m_ObjectID);
        dest.writeString(this.m_ParentID);
        dest.writeString(this.m_Type);
        dest.writeString(this.m_Title);
        dest.writeString(this.m_Uri);
        dest.writeString(this.m_Didl);
        dest.writeLong(this.m_Size);
    }

    protected MediaObject(Parcel in) {
        this.m_ObjectID = in.readString();
        this.m_ParentID = in.readString();
        this.m_Type = in.readString();
        this.m_Title = in.readString();
        this.m_Uri = in.readString();
        this.m_Didl = in.readString();
        this.m_Size = in.readLong();
    }

    public static final Parcelable.Creator<MediaObject> CREATOR = new Parcelable.Creator<MediaObject>() {
        public MediaObject createFromParcel(Parcel source) {
            return new MediaObject(source);
        }

        public MediaObject[] newArray(int size) {
            return new MediaObject[size];
        }
    };
}
