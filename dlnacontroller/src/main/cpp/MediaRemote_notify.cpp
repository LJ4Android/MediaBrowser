//
// Created by wh on 2017/12/10.
//
#include <PltMediaItem.h>
#include "MediaRemote_notify.h"

#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, "sinadlna_jni", __VA_ARGS__))

JNIMediaRemoteEventNotify::JNIMediaRemoteEventNotify(JavaVM *jvm_, jobject thiz,
                                                     jobject weak_thiz) {
    bool isAttached = false;
    JNIEnv *env = NULL;
    jvm = jvm_;
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        jint res = jvm->AttachCurrentThread(&env, NULL);
        if ((res < 0) || !env) {
            return;
        }
        isAttached = true;
    }
    jclass clazz = env->GetObjectClass(thiz);
    if (clazz == NULL) {
        return;
    }
    mClass = (jclass) env->NewGlobalRef(clazz);

    mObject = env->NewGlobalRef(weak_thiz);

    this->post_device_event = env->GetStaticMethodID(clazz, "postDeviceChange",
                                                     "(Ljava/lang/Object;ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    this->post_browser_event = env->GetStaticMethodID(clazz, "postMSMediaBrowser",
                                                      "(Ljava/lang/Object;ILjava/util/ArrayList;)V");
    this->post_mr_response_event = env->GetStaticMethodID(clazz, "postMRActionResponse",
                                                          "(Ljava/lang/Object;III)V");
    this->post_mr_request_event = env->GetStaticMethodID(clazz, "postMRActionRequest",
                                                         "(Ljava/lang/Object;ILjava/lang/String;Ljava/lang/String;)V");
    this->post_mr_state_event = env->GetStaticMethodID(clazz, "postMRStateVariablesChanged",
                                                       "(Ljava/lang/Object;II)V");
    this->post_mr_media_event = env->GetStaticMethodID(clazz,"postMRMediaDataChanged",
                                                       "(Ljava/lang/Object;ILjava/lang/String;)V");

    if (isAttached) {
        jvm->DetachCurrentThread();
    }
}

JNIMediaRemoteEventNotify::~JNIMediaRemoteEventNotify() {
    bool isAttached = false;
    JNIEnv *env = NULL;
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        jint res = jvm->AttachCurrentThread(&env, NULL);
        if ((res < 0) || !env) {
            return;
        }
        isAttached = true;
    }
    env->DeleteGlobalRef(mObject);
    env->DeleteGlobalRef(mClass);

    if (isAttached) {
        jvm->DetachCurrentThread();
    }
}

void JNIMediaRemoteEventNotify::postDeviceChange(int event, const char *uuid, const char *name,
                                                 const char *type) {
    if (this->post_device_event == NULL) {
        return;
    }
    bool isAttached = false;
    JNIEnv *env = NULL;
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        jint res = jvm->AttachCurrentThread(&env, NULL);
        if ((res < 0) || !env) {
            return;
        }
        isAttached = true;
    }
    jstring j_uuid=env->NewStringUTF(uuid);
    jstring j_name=env->NewStringUTF(name);
    jstring j_type=env->NewStringUTF(type);
    env->CallStaticVoidMethod(mClass, this->post_device_event, mObject,
                              event, j_uuid, j_name,j_type);
    env->DeleteLocalRef(j_uuid);
    env->DeleteLocalRef(j_name);
    env->DeleteLocalRef(j_type);
    if (isAttached) {
        jvm->DetachCurrentThread();
    }
}

void
JNIMediaRemoteEventNotify::postMSMediaBrowser(int result, PLT_MediaObjectListReference objectList) {
    if (this->post_browser_event == NULL) {
        return;
    }
    bool isAttached = false;
    JNIEnv *env = NULL;
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        jint res = jvm->AttachCurrentThread(&env, NULL);
        if ((res < 0) || !env) {
            return;
        }
        isAttached = true;
    }
    LOGI("OnContainerChanged objectList IsNull=%d", objectList.IsNull());
    if (!objectList.IsNull()) {
        jclass cls_ArrayList = env->FindClass("java/util/ArrayList");
        jmethodID construct = env->GetMethodID(cls_ArrayList, "<init>", "()V");
        jobject obj_ArrayList = env->NewObject(cls_ArrayList, construct, "");
        jmethodID arrayList_add = env->GetMethodID(cls_ArrayList, "add", "(Ljava/lang/Object;)Z");
        jclass mediaObjectClass = env->FindClass("com/lj/dlnacontroller/api/MediaObject");
        jmethodID mediaObjectMethod = env->GetMethodID(mediaObjectClass, "<init>", "()V");
        jfieldID m_ObjectID = env->GetFieldID(mediaObjectClass, "m_ObjectID", "Ljava/lang/String;");
        jfieldID m_ParentID = env->GetFieldID(mediaObjectClass, "m_ParentID", "Ljava/lang/String;");
        jfieldID m_Type = env->GetFieldID(mediaObjectClass, "m_Type", "Ljava/lang/String;");
        jfieldID m_Title = env->GetFieldID(mediaObjectClass, "m_Title", "Ljava/lang/String;");
        jfieldID m_Didl = env->GetFieldID(mediaObjectClass, "m_Didl", "Ljava/lang/String;");
        jfieldID m_Uri = env->GetFieldID(mediaObjectClass, "m_Uri", "Ljava/lang/String;");
        jfieldID m_Size = env->GetFieldID(mediaObjectClass, "m_Size", "J"); //J long //Z boolean //I int

        NPT_List<PLT_MediaObject *>::Iterator item = objectList->GetFirstItem();
        while (item) {
            jobject mediaObject = env->NewObject(mediaObjectClass, mediaObjectMethod);
            jstring j_ObjectID = env->NewStringUTF((*item)->m_ObjectID.GetChars());
            jstring j_ParentID = env->NewStringUTF((*item)->m_ParentID.GetChars());
            jstring j_Type = env->NewStringUTF((*item)->m_ObjectClass.type.GetChars());
            jstring j_Title = env->NewStringUTF((*item)->m_Title.GetChars());
            jstring j_Didl = env->NewStringUTF((*item)->m_Didl.GetChars());
            jstring j_Uri = NULL;
            env->SetObjectField(mediaObject, m_ObjectID, j_ObjectID);
            env->SetObjectField(mediaObject, m_ParentID,j_ParentID );
            env->SetObjectField(mediaObject, m_Type, j_Type);
            env->SetObjectField(mediaObject, m_Title, j_Title);
            env->SetObjectField(mediaObject, m_Didl, j_Didl);
            if (!(*item)->IsContainer()) {
                j_Uri = env->NewStringUTF((*item)->m_Resources[0].m_Uri.GetChars());
                env->SetObjectField(mediaObject, m_Uri, j_Uri);
                env->SetLongField(mediaObject, m_Size, (*item)->m_Resources[0].m_Size);
            }
            env->CallBooleanMethod(obj_ArrayList, arrayList_add, mediaObject);//CallBooleanMethod 函数返回值为boolean
            env->DeleteLocalRef(mediaObject);
            env->DeleteLocalRef(j_ObjectID);
            env->DeleteLocalRef(j_ParentID);
            env->DeleteLocalRef(j_Type);
            env->DeleteLocalRef(j_Title);
            env->DeleteLocalRef(j_Didl);
            env->DeleteLocalRef(j_Uri);
            ++item;
        }
        env->CallStaticVoidMethod(mClass, this->post_browser_event, mObject, result, obj_ArrayList);
        env->DeleteLocalRef(cls_ArrayList);
        env->DeleteLocalRef(obj_ArrayList);
        env->DeleteLocalRef(mediaObjectClass);
    }else{
        env->CallStaticVoidMethod(mClass, this->post_browser_event, mObject, result, NULL);
    }

    if (isAttached) {
        jvm->DetachCurrentThread();
    }
}

void JNIMediaRemoteEventNotify::postMRActionResponse(int event, int result, int value) {
    if (this->post_mr_response_event == NULL) {
        return;
    }
    bool isAttached = false;
    JNIEnv *env = NULL;
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        jint res = jvm->AttachCurrentThread(&env, NULL);
        if ((res < 0) || !env) {
            return;
        }
        isAttached = true;
    }
    env->CallStaticVoidMethod(mClass, this->post_mr_response_event, mObject,
                              event, result, value);

    if (isAttached) {
        jvm->DetachCurrentThread();
    }
}

void
JNIMediaRemoteEventNotify::postMRActionRequest(int event, const char *value, const char *data) {
    if (this->post_mr_request_event == NULL) {
        return;
    }
    bool isAttached = false;
    JNIEnv *env = NULL;
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        jint res = jvm->AttachCurrentThread(&env, NULL);
        if ((res < 0) || !env) {
            return;
        }
        isAttached = true;
    }
    jstring j_value = env->NewStringUTF(value);
    jstring j_data = env->NewStringUTF(data);
    LOGI("postMRActionRequest event = %d,value = %s,data = %s",event,value,data);
    env->CallStaticVoidMethod(mClass, this->post_mr_request_event, mObject,
                              event, j_value,j_data);
    env->DeleteLocalRef(j_value);
    env->DeleteLocalRef(j_data);

    if (isAttached) {
        jvm->DetachCurrentThread();
    }
}

void JNIMediaRemoteEventNotify::postMRStateVariablesChanged(int event, int value) {
    if (this->post_mr_state_event == NULL) {
        return;
    }
    bool isAttached = false;
    JNIEnv *env = NULL;
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        jint res = jvm->AttachCurrentThread(&env, NULL);
        if ((res < 0) || !env) {
            return;
        }
        isAttached = true;
    }
    env->CallStaticVoidMethod(mClass, this->post_mr_state_event, mObject, event, value);
    if (isAttached) {
        jvm->DetachCurrentThread();
    }
}

void JNIMediaRemoteEventNotify::postMRMediaDataChanged(int event, const char *value) {
    if (this->post_mr_media_event == NULL) {
        return;
    }
    bool isAttached = false;
    JNIEnv *env = NULL;
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        jint res = jvm->AttachCurrentThread(&env, NULL);
        if ((res < 0) || !env) {
            return;
        }
        isAttached = true;
    }
    jstring j_value = env->NewStringUTF(value);
    env->CallStaticVoidMethod(mClass, this->post_mr_media_event, mObject, event, j_value);
    env->DeleteLocalRef(j_value);
    if (isAttached) {
        jvm->DetachCurrentThread();
    }
}
