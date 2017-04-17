#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <GLES/gl.h>
#include <GLES/egl.h>
#include <gbm.h>
#include <xf86drmMode.h>

#define LOGD(...) fprintf(stdout, __VA_ARGS__); printf("\n")
#define LOGE(...) fprintf(stderr, __VA_ARGS__); printf("\n")

static const char drm_device_name[] = "/dev/dri/card0";

const EGLint attribs[] = {
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_DEPTH_SIZE, 24,
    EGL_STENCIL_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_NONE
};

struct kms {
    drmModeConnector *connector;
    drmModeEncoder *encoder;
    drmModeModeInfo mode;
    uint32_t fb_id;
};

static EGLBoolean
setup_kms(int fd, struct kms *kms)
{
    drmModeRes *resources;
    drmModeConnector *connector;
    drmModeEncoder *encoder;
    int i;

    resources = drmModeGetResources(fd);
    if (!resources) {
        fprintf(stderr, "drmModeGetResources failed\n");
        return EGL_FALSE;
    }

    for (i = 0; i < resources->count_connectors; i++) {
        connector = drmModeGetConnector(fd, resources->connectors[i]);
        if (connector == NULL)
            continue;

        if (connector->connection == DRM_MODE_CONNECTED &&
            connector->count_modes > 0)
            break;

        drmModeFreeConnector(connector);
    }

    if (i == resources->count_connectors) {
        fprintf(stderr, "No currently active connector found.\n");
        return EGL_FALSE;
    }

    for (i = 0; i < resources->count_encoders; i++) {
        encoder = drmModeGetEncoder(fd, resources->encoders[i]);

        if (encoder == NULL)
            continue;

        if (encoder->encoder_id == connector->encoder_id)
            break;

        drmModeFreeEncoder(encoder);
    }

    kms->connector = connector;
    kms->encoder = encoder;
    kms->mode = connector->modes[0];

    return EGL_TRUE;
}

void render_one_frame() {
    glViewport(0, 0, (GLint) 800, (GLint) 1280);
    glClear(GL_COLOR_BUFFER_BIT /*| GL_DEPTH_BUFFER_BIT*/);
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glFinish();
}

int main() {
    std::cout << "Hello, World!" << std::endl;

    int fd, ret;
    EGLDisplay eglDisplay;
    EGLConfig eglConfig;
    EGLContext eglContext;
    EGLSurface eglSurface;
    EGLint verMajor, verMinor, n;
    EGLint numConfigs;
    EGLint attrs[3] = { EGL_DEPTH_SIZE, 16, EGL_NONE };
    struct gbm_device *gbm;
    const char *ver;
    struct kms kms;
    struct gbm_surface *gbmSurface;
    struct gbm_bo *bo;
    uint32_t handle, stride;
    drmModeCrtcPtr saved_crtc;

    // 1. Get Display
    fd = open(drm_device_name, O_RDWR);
    if (fd < 0) {
        /* Probably permissions error */
        LOGE("couldn't open %s, skipping", drm_device_name);
        return -1;
    }

    gbm = gbm_create_device(fd);
    if (gbm == NULL) {
        LOGE("couldn't create gbm device");
        close(fd);
        return -1;
    }

    eglDisplay = eglGetDisplay(gbm); // not EGL_DEFAULT_DISPLAY !
    if( eglDisplay == EGL_NO_DISPLAY || eglGetError() != EGL_SUCCESS ) {
        LOGE("getdisplay error !");
        return eglGetError();
    }

    // 2. Initialize EGL
    if ( eglInitialize(eglDisplay, &verMajor, &verMinor) == EGL_FALSE || eglGetError() != EGL_SUCCESS ) {
        LOGE("egl init error ! %d", eglGetError());
        return eglGetError();
    }

    ver = eglQueryString(eglDisplay, EGL_VERSION);

    if ( eglGetConfigs( eglDisplay, NULL, 0, &numConfigs) == EGL_FALSE || eglGetError() != EGL_SUCCESS ) {
        std::cerr << "getdisplay error !" << std::endl;
        return eglGetError();
    }

    LOGD("* EGL_VERSION = %s (have %d configs)", ver, numConfigs);

    if (!setup_kms(fd, &kms)) {
        LOGE("setup kms failed !");
        return -1;
    }
    eglBindAPI(EGL_OPENGL_ES_API);

    // 3. Choose Config
    if (!eglChooseConfig(eglDisplay, attribs, &eglConfig, 1, &n) || n != 1) {
        LOGE("failed to choose argb config");
        return eglGetError();
    }

    // 4. Create Surface
    gbmSurface = gbm_surface_create(gbm, kms.mode.hdisplay, kms.mode.vdisplay,
                            GBM_FORMAT_XRGB8888,
                            GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
    fprintf(stderr, "%s %d gbmSurface=%p\n",__func__,__LINE__, gbmSurface);
    eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, gbmSurface, NULL);


    // 5. Create Context
    eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, NULL);
    if (eglContext == NULL) {
        LOGE("failed to create context");
        return eglGetError();
    }

    if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
        LOGE("failed to make context current");
        return eglGetError();
    }

    //glViewport(0, 0, (GLint) 800, (GLint) 1280);
    glClearColor(1.0, 0.5, 0.5, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    eglSwapBuffers(eglDisplay, eglSurface);

    bo = gbm_surface_lock_front_buffer(gbmSurface);
    handle = gbm_bo_get_handle(bo).u32;
    stride = gbm_bo_get_stride(bo);
    int width = gbm_bo_get_width(bo);
    int height = gbm_bo_get_height(bo);

    printf("handle=%d, stride=%d rect=%dx%d\n", handle, stride, width, height);

    /*void *buffer = malloc(800*1280 * 4);
    memset(buffer, 0xf0, 800*1280*4);
    gbm_bo_write(bo, buffer, 800*1280*4);
    free(buffer);*/

    ret = drmModeAddFB(fd, kms.mode.hdisplay, kms.mode.vdisplay,
                       24, 32, stride, handle, &kms.fb_id);
    if (ret) {
        LOGE("failed to create fb");
        //goto rm_fb;
        return -1;
    }

    saved_crtc = drmModeGetCrtc(fd, kms.encoder->crtc_id);
    if (saved_crtc == NULL) {
        LOGE("failed to crtc: %m");
        return -1;
    }

    ret = drmModeSetCrtc(fd, kms.encoder->crtc_id, kms.fb_id, 0, 0,
                         &kms.connector->connector_id, 1, &kms.mode);
    if (ret) {
        LOGE("failed to set mode: %m");
        //goto free_saved_crtc;
        return -1;
    }

    getchar();

    ret = drmModeSetCrtc(fd, saved_crtc->crtc_id, saved_crtc->buffer_id,
                         saved_crtc->x, saved_crtc->y,
                         &kms.connector->connector_id, 1, &saved_crtc->mode);
    if (ret) {
        LOGE("failed to restore crtc: %m");
    }

    return 0;
}