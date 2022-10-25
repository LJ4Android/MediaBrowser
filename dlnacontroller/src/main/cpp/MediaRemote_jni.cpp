//
// Created by wh on 2017/12/10.
//

#include "MediaRemote_jni.h"

#ifndef NELEM
# define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

struct MediaRemoteClassFields{
    jfieldID mNativeContext;
};

static const char* const kClassPathName 	= 	"com/lj/dlnacontroller/api/MediaRemote";

static MediaRemoteClassFields gMediaRemoteClassFields={0};
static pthread_mutex_t* gMediaRemoteObjectLock=NULL;
static JavaVM* gJavaVM = NULL;

#ifdef __cplusplus
extern "C" {
#endif
/*
 * Register native JNI-callable methods.
 *
 * "className" looks like "java/lang/String".
 */
int jniRegisterNativeMethods(JNIEnv* env, const char* className,
                             const JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;
    clazz = env->FindClass( className);
    if (clazz == NULL) {
        NPT_LOG_INFO_1("Native registration unable to find class '%s'\n", className);
        return NPT_FAILURE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        NPT_LOG_INFO_1("RegisterNatives failed for '%s'\n", className);
        return NPT_FAILURE;
    }
    return NPT_SUCCESS;
}

/*
 * Throw an exception with the specified class and an optional message.
 */
int jniThrowException(JNIEnv* env, const char* className, const char* msg)
{
    jclass exceptionClass;

    exceptionClass = env->FindClass( className);
    if (exceptionClass == NULL) {
        NPT_LOG_INFO_1("Unable to find exception class %s\n", className);
        return NPT_FAILURE;
    }

    if (env->ThrowNew( exceptionClass, msg) != JNI_OK) {
        NPT_LOG_INFO_2("Failed throwing '%s' '%s'\n", className, msg);
        return NPT_FAILURE;
    }
    return NPT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

static pthread_mutex_t* CreateMutex()
{
    pthread_mutex_t* mtx = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    if(mtx==NULL){
        return NULL;
    }
    int res = pthread_mutex_init(mtx,NULL);
    if(NPT_SUCCESS!=res){
        return NULL;
    }
    return mtx;
}

static int LockMutex(pthread_mutex_t* mtx)
{
    return pthread_mutex_lock(mtx);
}

static int UnlockMutex(pthread_mutex_t* mtx)
{
    return pthread_mutex_unlock(mtx);
}

static int DestroyMutex(pthread_mutex_t* mtx)
{
    if(mtx){
        pthread_mutex_destroy(mtx);
        free(mtx);
        return NPT_SUCCESS;
    }
    return NPT_FAILURE;
}

static MediaRemote* getMediaRemote(JNIEnv* env, jobject thiz)
{
    if(gMediaRemoteObjectLock)
        LockMutex(gMediaRemoteObjectLock);
    MediaRemote* p = (MediaRemote*)env->GetLongField(thiz, gMediaRemoteClassFields.mNativeContext);
    if(gMediaRemoteObjectLock)
        UnlockMutex(gMediaRemoteObjectLock);
    return p;
}

static MediaRemote* setMediaRemote(JNIEnv* env, jobject thiz, MediaRemote* context)
{
    if(gMediaRemoteObjectLock)
        LockMutex(gMediaRemoteObjectLock);
    MediaRemote* old = (MediaRemote*)env->GetLongField(thiz, gMediaRemoteClassFields.mNativeContext);
    env->SetLongField(thiz, gMediaRemoteClassFields.mNativeContext, (long)context);
    if(gMediaRemoteObjectLock)
        UnlockMutex(gMediaRemoteObjectLock);
    return old;
}

static void _setup(JNIEnv *env,jobject thiz, jobject weak_thiz) {

    NPT_LOG_INFO_1("%s into",__FUNCTION__);

    jclass clazz;

    clazz = env->FindClass(kClassPathName);
    if (clazz == NULL) {
        return;
    }

    gMediaRemoteClassFields.mNativeContext = env->GetFieldID(clazz, "mNativeContext", "J");
    if (gMediaRemoteClassFields.mNativeContext == NULL) {
        return;
    }

    if(gMediaRemoteObjectLock==NULL){
        gMediaRemoteObjectLock =  CreateMutex();
    }

    MediaRemote* remote=MediaRemoteHelper::create();
    if(NULL==remote){
        jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
        return;
    }
    int ret=remote->setup(gJavaVM,thiz,weak_thiz);
    if(NPT_SUCCESS == ret)
    {
        setMediaRemote(env,thiz,remote);
    }

}

static void _release(JNIEnv *env, jobject thiz) {
    MediaRemote* remote = setMediaRemote(env, thiz, 0);
    if (remote != NULL) {
        remote->release();
        MediaRemoteHelper::destroy(remote);
    }
}

static void _finalize(JNIEnv * env, jobject thiz){
    _release(env,thiz);
}

static int setDMR(JNIEnv * env, jobject thiz, jstring uuid) {

    MediaRemote* remote=getMediaRemote(env,thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return NPT_FAILURE;
    }

    if (uuid == NULL) {
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return NPT_FAILURE;
    }

    const char *tmp = env->GetStringUTFChars(uuid, NULL);
    if (tmp == NULL) {  // Out of memory
        jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
        return NPT_FAILURE;
    }

    int ret = remote->setDMR(tmp);

    env->ReleaseStringUTFChars(uuid, tmp);
    tmp = NULL;
    return ret;
}

static jstring getDMR(JNIEnv * env, jobject thiz) {
    MediaRemote* remote = getMediaRemote(env, thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return NULL;
    }
    const char* uuid = remote->getDMR();

    if(uuid){
        return env->NewStringUTF(uuid);
    }else{
        return NULL;
    }
}

static int setDMS(JNIEnv * env, jobject thiz, jstring uuid) {

    MediaRemote* remote=getMediaRemote(env,thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return NPT_FAILURE;
    }

    if (uuid == NULL) {
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return NPT_FAILURE;
    }

    const char *tmp = env->GetStringUTFChars(uuid, NULL);
    if (tmp == NULL) {  // Out of memory
        jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
        return NPT_FAILURE;
    }

    int ret = remote->setDMS(tmp);

    env->ReleaseStringUTFChars(uuid, tmp);
    tmp = NULL;
    return ret;
}

static jstring getDMS(JNIEnv * env, jobject thiz) {
    MediaRemote* remote = getMediaRemote(env, thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return NULL;
    }
    const char* uuid = remote->getDMS();

    if(uuid){
        return env->NewStringUTF(uuid);
    }else{
        return NULL;
    }
}

static int startMediaController(JNIEnv * env, jobject thiz) {
    int ret;
    MediaRemote* remote = getMediaRemote(env, thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return NPT_FAILURE;
    }

    ret = remote->startMediaController();

    return ret;
}

static int stopMediaController(JNIEnv * env, jobject thiz) {
    int ret;
    MediaRemote* remote = getMediaRemote(env, thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return NPT_FAILURE;
    }

    ret = remote->stopMediaController();
    return ret;
}

static int startMediaRender(JNIEnv * env, jobject thiz,jstring friendly_name,jstring uuid) {
    int ret;
    MediaRemote* remote = getMediaRemote(env, thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return NPT_FAILURE;
    }

    if (friendly_name == NULL) {
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return NPT_FAILURE;
    }

    if (uuid == NULL) {
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return NPT_FAILURE;
    }

    const char *name = env->GetStringUTFChars(friendly_name, NULL);
    const char *uid =  env->GetStringUTFChars(uuid,NULL);
    if (name == NULL) {  // Out of memory
        jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
        return NPT_FAILURE;
    }
    if (uid == NULL) {  // Out of memory
        jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
        return NPT_FAILURE;
    }
    ret = remote->startMediaRender(name,uid);
    env->ReleaseStringUTFChars(uuid, uid);
    env->ReleaseStringUTFChars(friendly_name, name);
    uid=NULL;
    name = NULL;
    return ret;
}

static int stopMediaRender(JNIEnv * env, jobject thiz) {
    int ret;
    MediaRemote* remote = getMediaRemote(env, thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return NPT_FAILURE;
    }
    ret = remote->stopMediaRender();
    return ret;
}

static int startMediaServer(JNIEnv * env, jobject thiz,jstring path,jstring friendly_name,jstring uuid) {
    int ret;
    MediaRemote* remote = getMediaRemote(env, thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return NPT_FAILURE;
    }

    if(path == NULL){
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return NPT_FAILURE;
    }

    if (friendly_name == NULL) {
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return NPT_FAILURE;
    }

    if (uuid == NULL) {
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return NPT_FAILURE;
    }

    const char *name = env->GetStringUTFChars(friendly_name, NULL);
    const char *uid =  env->GetStringUTFChars(uuid,NULL);
    const char *pth =  env->GetStringUTFChars(path,NULL);
    if (name == NULL) {  // Out of memory
        jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
        return NPT_FAILURE;
    }
    if (uid == NULL) {  // Out of memory
        jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
        return NPT_FAILURE;
    }
    if(pth == NULL){
        jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
        return NPT_FAILURE;
    }
    ret = remote->startMediaServer(pth,name,uid);
    env->ReleaseStringUTFChars(uuid, uid);
    env->ReleaseStringUTFChars(friendly_name, name);
    env->ReleaseStringUTFChars(path,pth);
    uid=NULL;
    name = NULL;
    pth=NULL;
    return ret;
}

static int stopMediaServer(JNIEnv * env, jobject thiz) {
    int ret;
    MediaRemote* remote = getMediaRemote(env, thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return NPT_FAILURE;
    }
    ret = remote->stopMediaServer();
    return ret;
}

static int DoBrowse(JNIEnv * env, jobject thiz,jstring object_id,jboolean metadata){
    int ret;
    MediaRemote* remote = getMediaRemote(env, thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return NPT_FAILURE;
    }

    if(object_id == NULL){
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return NPT_FAILURE;
    }
    const char *obj_id = env->GetStringUTFChars(object_id, NULL);
    ret = remote->DoBrowse(obj_id,metadata);
    env->ReleaseStringUTFChars(object_id, obj_id);
    obj_id=NULL;
    return ret;
}

static int responseMRGenaEvent(JNIEnv *env, jobject thiz, jint cmd, jstring value, jstring data){
    int ret;
    MediaRemote* remote = getMediaRemote(env, thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return NPT_FAILURE;
    }

    if(value == NULL){
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return NPT_FAILURE;
    }

    if (data == NULL) {
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return NPT_FAILURE;
    }

    const char *val = env->GetStringUTFChars(value, NULL);
    const char *dat =  env->GetStringUTFChars(data,NULL);
    if (val == NULL) {  // Out of memory
        jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
        return NPT_FAILURE;
    }
    if (dat == NULL) {  // Out of memory
        jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
        return NPT_FAILURE;
    }
    LOGI("cmd = %d,val = %s",cmd,val);
    ret = remote->responseMRGenaEvent(cmd,val,dat);
    env->ReleaseStringUTFChars(value, val);
    env->ReleaseStringUTFChars(data, dat);
    val=NULL;
    dat = NULL;
    return ret;
};

static void open(JNIEnv * env, jobject thiz, jstring url,jstring didl) {

    MediaRemote* remote = getMediaRemote(env, thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    if (url == NULL ) { //didl canbe NULL
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return;
    }

    const char *tmpUrl = env->GetStringUTFChars(url, NULL);
    if (tmpUrl == NULL) {  // Out of memory
        jniThrowException(env, "java/lang/RuntimeException", "url param Out of memory");
        return;
    }
    const char *tmpDidl = NULL;
    if(didl!=NULL){
        tmpDidl = env->GetStringUTFChars(didl, NULL);
        if (tmpDidl == NULL) {  // Out of memory
            jniThrowException(env, "java/lang/RuntimeException", "didl param Out of memory");
            return;
        }
    }

    int ret = remote->open(tmpUrl,tmpDidl);

    env->ReleaseStringUTFChars(url, tmpUrl);
    if(didl!=NULL){
        env->ReleaseStringUTFChars(didl, tmpDidl);
    }
}

static void play(JNIEnv * env, jobject thiz) {
    MediaRemote* remote = getMediaRemote(env, thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    int ret = remote->play();
}

static void pause(JNIEnv * env, jobject thiz) {
    MediaRemote* remote = getMediaRemote(env, thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    int ret = remote->pause();
}

static void stop(JNIEnv * env, jobject thiz) {
    MediaRemote* remote = getMediaRemote(env, thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    int ret = remote->stop();
}

static void seek(JNIEnv * env, jobject thiz, jint msec) {
    MediaRemote* remote = getMediaRemote(env, thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    if(msec < 0){
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return;
    }
    int ret = remote->seek(msec);
}

static void setMute(JNIEnv * env, jobject thiz, jboolean mute) {
    MediaRemote* remote = getMediaRemote(env, thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    int ret = remote->setMute(mute);
}

static void getMute(JNIEnv * env, jobject thiz) {
    MediaRemote* remote = getMediaRemote(env, thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    int ret = remote->getMute();
}

static void setVolume(JNIEnv * env, jobject thiz, jint vol) {
    MediaRemote* remote = getMediaRemote(env, thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    if(vol < 0){
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return;
    }

    int ret = remote->setVolume(vol);
}

static void getVolume(JNIEnv * env, jobject thiz) {
    MediaRemote* remote = getMediaRemote(env, thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    int ret = remote->getVolume();
}

static jint getVolumeMin(JNIEnv * env, jobject thiz){
    MediaRemote* remote = getMediaRemote(env, thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return NPT_FAILURE;
    }
    int volMin = -1, volMax = -1;
    int ret = remote->getVolumeRange(volMin,volMax);

    return volMin;
}

static jint getVolumeMax(JNIEnv * env, jobject thiz){
    MediaRemote* remote = getMediaRemote(env, thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return NPT_FAILURE;
    }
    int volMin = -1, volMax = -1;
    int ret = remote->getVolumeRange(volMin,volMax);

    return volMax;
}

static void getDuration(JNIEnv * env, jobject thiz) {
    MediaRemote* remote = getMediaRemote(env, thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    int ret = remote->getDuration();
}

static void getPosition(JNIEnv * env, jobject thiz) {
    MediaRemote* remote = getMediaRemote(env, thiz);
    if (remote == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return;
    }

    int ret = remote->getPosition();
}

static JNINativeMethod gMethods[] = {
        {"_setup",         		     "(Ljava/lang/Object;)V",         			(void *)_setup},
        {"_release",        	     "()V",            								(void *)_release},
        {"_finalize",     		     "()V",                              			(void *)_finalize},
        {"setDMR",                  "(Ljava/lang/String;)I",            			(void *)setDMR},
        {"getDMR",                  "()Ljava/lang/String;",                      (void *)getDMR},
        {"setDMS",                  "(Ljava/lang/String;)I",            			(void *)setDMS},
        {"getDMS",                  "()Ljava/lang/String;",                      (void *)getDMS},
        {"startMediaController",   "()I",            						        (void *)startMediaController},
        {"stopMediaController",    "()I",                                        (void *)stopMediaController},
        {"startMediaRender", 	     "(Ljava/lang/String;Ljava/lang/String;)I",	(void *)startMediaRender},
        {"stopMediaRender",    	 "()I",    					                    (void *)stopMediaRender},
        {"startMediaServer",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I",  (void *)startMediaServer},
        {"stopMediaServer",        "()I",                              			(void *)stopMediaServer},
        {"DoBrowse",                "(Ljava/lang/String;Z)I",                    (void *)DoBrowse},
        {"responseMRGenaEvent",   "(ILjava/lang/String;Ljava/lang/String;)I", (void *)responseMRGenaEvent},
        {"open",            	    "(Ljava/lang/String;Ljava/lang/String;)V",  (void *)open},
        {"play",            	    "()V",                              			(void *)play},
        {"pause",            	    "()V",                              			(void *)pause},
        {"stop",            	    "()V",                              			(void *)stop},
        {"seek",            	    "(I)V",                              			(void *)seek},
        {"setMute",                	"(Z)V",                              			(void *)setMute},
        {"getMute",            	    "()V",                              			(void *)getMute},
        {"getVolumeMin",          "()I",                              			    (void *)getVolumeMin},
        {"getVolumeMax",          "()I",                              			    (void *)getVolumeMax},
        {"setVolume",             "(I)V",                              			(void *)setVolume},
        {"getVolume",             "()V",                              			    (void *)getVolume},
        {"getDuration",           "()V",                              			    (void *)getDuration},
        {"getPosition",           "()V",                              			    (void *)getPosition},
};

/*----------------------------------------------------------------------
|    JNI_OnLoad
+---------------------------------------------------------------------*/
jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    if(vm==NULL){
        goto bail;
    }

    gJavaVM = vm;

    if ( vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        goto bail;
    }
    if(env == NULL){
        goto bail;
    }

    NPT_LogManager::GetDefault().Configure("plist:.level=FINE;.handlers=ConsoleHandler;.ConsoleHandler.outputs=2;.ConsoleHandler.colors=false;.ConsoleHandler.filter=59");

    if (jniRegisterNativeMethods(env,kClassPathName, gMethods, NELEM(gMethods)) < 0) {
        goto bail;
    }
    // success -- return valid version number
    result = JNI_VERSION_1_4;
    bail:
    return result;
}


void JNI_OnUnload(JavaVM* vm, void* reserved)
{

    if(gMediaRemoteObjectLock!=NULL){
        DestroyMutex(gMediaRemoteObjectLock);
        gMediaRemoteObjectLock = NULL;
    }

}