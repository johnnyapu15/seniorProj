#include <android/log.h>
#include <jni.h>
#include "HSVFilter.h"

#include "patternMatch/patternMatchClass.h"

using namespace cv;
using namespace std;


extern "C" {

HSVFilter filter;
patternMatch* pm = NULL;
JNIEXPORT void JNICALL
Java_com_example_dlscj_dash_Dash_TouchCallback(JNIEnv *env, jobject instance,
                                               jint x, jint y) {
	filter.SelectColor((int)x, (int)y);
}

JNIEXPORT jboolean JNICALL
Java_com_example_dlscj_dash_Dash_HSVFilter(JNIEnv *env, jobject instance,
                                           jlong matAddrInput,
                                           jlong matAddrResult,
                                           jdoubleArray bbarr) {

	return filter.FindObj_JNI(env, instance, matAddrInput, matAddrResult, bbarr);
}

JNIEXPORT void JNICALL
Java_com_example_dlscj_dash_Dash_SetRange(JNIEnv *env, jobject instance, jintArray min, jintArray max){
    filter.SetRange(env, instance, min, max);
}

JNIEXPORT void JNICALL
Java_com_example_dlscj_dash_Dash_GetMin(JNIEnv *env, jobject instance, jintArray min){
    int len;
    len = env->GetArrayLength(min);
    jint* bb_buf = (jint *) malloc(sizeof(jint) * len);


    int* buf = filter.GetMinScalar();
    if(buf == NULL){
        bb_buf[0] = 0;
        bb_buf[1] = 0;
        bb_buf[2] = 0;
    }
    else{
        bb_buf[0] = buf[0];
        bb_buf[1] = buf[1];
        bb_buf[2] = buf[2];
    }
    env->SetIntArrayRegion(min, 0, len, bb_buf);
    free(bb_buf);
}

JNIEXPORT void JNICALL
Java_com_example_dlscj_dash_Dash_GetMax(JNIEnv *env, jobject instance, jintArray max){
    int len;
    len = env->GetArrayLength(max);
    jint* bb_buf = (jint *) malloc(sizeof(jint) * len);


    int* buf = filter.GetMaxScalar();
    if(buf == NULL){
        bb_buf[0] = 0;
        bb_buf[1] = 0;
        bb_buf[2] = 0;
    }
    else{
        bb_buf[0] = buf[0];
        bb_buf[1] = buf[1];
        bb_buf[2] = buf[2];
    }
    env->SetIntArrayRegion(max, 0, len, bb_buf);
    free(bb_buf);
}


JNIEXPORT void JNICALL
Java_com_example_dlscj_dash_Dash_LineS2E(JNIEnv *env, jobject instance,
                                         jlong matAddrInput,
                                         jlong matAddrResult,
                                         jdouble s_x,
                                         jdouble s_y,
                                         jdouble e_x,
                                         jdouble e_y) {
    IplImage in_img(*(Mat *) matAddrInput);
    Mat &out_mat = *(Mat *) matAddrResult;

    cvCircle(&in_img, CvPoint((int) s_x, (int) s_y), 10, CvScalar(0, 233, 255), 20);
    cvCircle(&in_img, CvPoint((int) e_x, (int) e_y), 10, CvScalar(0, 233, 255), 20);
    cvLine(&in_img, CvPoint((int) s_x, (int) s_y), CvPoint((int) e_x, (int) e_y), CvScalar(0, 86, 102), 10);
    cvarrToMat(&in_img).copyTo(out_mat);
}

///////////////////////////////////////////////////////
//Functions for pattern recognition

JNIEXPORT void JNICALL
Java_com_example_dlscj_dash_Dash_initPatternMatch(JNIEnv *env, jobject instance){
    pm = new patternMatch();
}

JNIEXPORT void JNICALL
Java_com_example_dlscj_dash_Dash_initParam2Img(JNIEnv *env,jobject instance, jlong time){
    if (pm != NULL)
        pm->initParam2Img(time);
}

JNIEXPORT void JNICALL
Java_com_example_dlscj_dash_Dash_updateParam2Img(JNIEnv *env, jobject instance, jlong time, jfloat velo, jfloat angle){
    if (pm != NULL)
        pm->updateParam2Img(time, (float)velo, (float)angle);
}

JNIEXPORT void JNICALL
Java_com_example_dlscj_dash_Dash_getPredicted(JNIEnv *env, jobject instance, jstring method, jfloatArray confidences){
    if (pm != NULL) {
        float* tmpConfs = NULL;
        if (confidences != NULL)
            tmpConfs = new float[7];

        const char *tmpChar = (*env).GetStringUTFChars(method, NULL);
        std::string metStr(tmpChar);
        pm->getPredicted(metStr, tmpConfs);
        (*env).SetFloatArrayRegion(confidences, 0, 7, tmpConfs);
        free(tmpConfs);
    }
}

//It uses index of classes for pattern validation. ex) CIRCLE -> 0, N -> 1 ...
//CLASSES : {"CIRCLE", "N", "L", "RECT", "RS", "S", "INTERMEDIATE"};
//               0      1    2     3      4     5          6
JNIEXPORT jboolean JNICALL
Java_com_example_dlscj_dash_Dash_isValidPattern(JNIEnv *env, jobject instance, jint idx, jfloat threshold){
    if (pm != NULL) {
        bool valid = pm->isValid(idx, threshold);
        return valid;
    }
    return false;
}

JNIEXPORT jboolean JNICALL
Java_com_example_dlscj_dash_Dash_isTopPattern(JNIEnv *env, jobject instance, jint idx, jfloat threshold){
    if (pm != NULL) {
        bool valid = pm->isTop(idx);
        valid = valid & pm->isValid(idx, threshold);
        return valid;
    }
    return false;
}

JNIEXPORT void JNICALL
Java_com_example_dlscj_dash_Dash_initNN(JNIEnv *env, jobject instance, jstring path){
    if (pm != NULL) {
        const char *tmpChar = (*env).GetStringUTFChars(path, NULL);
        std::string pathStr(tmpChar);
        pm->initPatternNN(pathStr);
    }
}

JNIEXPORT jboolean JNICALL
Java_com_example_dlscj_dash_Dash_isTop3(JNIEnv *env, jobject instance, jint idx, jfloat threshold){
    if (pm != NULL) {
        int q = 0, w = 0, e = 0;
        pm->getThreeTop(q, w, e, threshold);
        return (q == idx || w == idx || e == idx);
    }
}

JNIEXPORT void JNICALL
Java_com_example_dlscj_dash_Dash_getImageFromParam(JNIEnv *env, jobject instance, jlong inputMat){
    if (pm != NULL) {
        //Mat m;
        pm->getImageFromParam(*(Mat *)inputMat, 1);
    }
}

JNIEXPORT jint JNICALL
Java_com_example_dlscj_dash_Dash_getPointNum(JNIEnv *env, jobject instance){
    if (pm != NULL) {
        return pm->getPointNum();
    }
    return -1;
}

}


