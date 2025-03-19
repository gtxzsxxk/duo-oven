
#define _GNU_SOURCE
#define CONTINOUS_DET_CAMERA	0	// 连续探测摄像头

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include <sys/mman.h>

#if CONTINOUS_DET_CAMERA
#include <pthread.h>
#endif

#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>

#include "uvctest.h"

#define _OPEN_TEST_FUNC_ 1

typedef enum pixformat{
    CVI_PIXFMT_YUYV_E,
    CVI_PIXFMT_MJPEG_E,
    CVI_PIXFMT_BUTT
}CVI_PIXFMT_E;

struct v4l2_bufsinfo{
	__u32 length;
	void *start;
};

#define REQ_BUFS_CNT 2

typedef struct cvi_uvc_host_ctx{
    int fd;
    CVI_PIXFMT_E fmt;
    pthread_t video_tid;
    volatile int video_exit;
    volatile int start_flag;
#ifdef _OPEN_TEST_FUNC_
    int testfd;
    char fname[256];
#endif
    struct v4l2_bufsinfo bufs[REQ_BUFS_CNT];
}CVI_UVC_HOST_CTX;

#define FMT_MAX_CNT 20
#define EVENT_MSG_BUF_SIZE (64 * 1024)

static int uvc_open(const char *dev)
{
	int fd = open(dev, O_RDWR);
	if(fd <= 0){
		printf("open error with %s", strerror(errno));
	}
	return fd;
}

static void uvc_close(int fd)
{
	if(fd > 0)
	{
		close(fd);
	}
}

static int uvc_get_capability(int fd)
{
	int video_dev_flag = 0;
	struct v4l2_capability cap;
	memset(&cap, 0x0, sizeof(struct v4l2_capability));
	int ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
	if(ret < 0)
	{
		printf("uvc_get_capability error with %s", strerror(errno));
		return -1;
	}

	if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE)
	{
		printf("support capture.\n");
		video_dev_flag++;
	}

	if ((cap.capabilities & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING)
	{
		printf("support streaming\n");
		video_dev_flag++;
	}

	if ((cap.capabilities & V4L2_CAP_EXT_PIX_FORMAT) == V4L2_CAP_EXT_PIX_FORMAT)
	{
		printf("support ext format\n");
		video_dev_flag++;
	}

	if(video_dev_flag == 3)
	{
        video_dev_flag = 0xFF;
	}

	printf("driver:\t\t%s\n",cap.driver);
	printf("card:\t\t%s\n",cap.card);
	printf("bus_info:\t%s\n",cap.bus_info);
	printf("version:\t%d\n",cap.version);
	printf("capabilities:\t%x\n",cap.capabilities);
	printf("\n");

	return video_dev_flag;
}


static int uvc_get_fmtdesc(int fd, struct v4l2_fmtdesc *pfmtdesc, struct v4l2_frmsizeenum *pframsize)
{
	struct v4l2_fmtdesc fmts;
	struct v4l2_frmsizeenum frmsize;
	memset(&fmts, 0x0, sizeof(struct v4l2_fmtdesc));
	fmts.index = 0;
	fmts.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	printf("Supportformat:\n");
	while(ioctl(fd, VIDIOC_ENUM_FMT, &fmts) != -1)
	{
		printf("\t%d.%s\n", fmts.index + 1, fmts.description);
		frmsize.pixel_format = fmts.pixelformat;
        frmsize.index = 0;
		while(ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) != -1)
		{
			if(frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
			{
				printf("\tdisc %dx%d\n", frmsize.discrete.width, frmsize.discrete.height);
			}
			else if (frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE)
			{
				printf("\tstep %dx%d\n", frmsize.stepwise.max_width, frmsize.stepwise.max_height);
				printf("\tstep %dx%d\n", frmsize.stepwise.min_width, frmsize.stepwise.min_height);
				printf("\tstep %dx%d\n", frmsize.stepwise.step_width, frmsize.stepwise.step_height);
			}
			memcpy(&pframsize[frmsize.index], &frmsize, sizeof(struct v4l2_frmsizeenum));
			frmsize.index++;
			if(fmts.index >= FMT_MAX_CNT * 2)
    		{
    			break;
    		}
		}
		memcpy(&pfmtdesc[fmts.index], &fmts, sizeof(struct v4l2_fmtdesc));
		fmts.index++;
		if(fmts.index >= FMT_MAX_CNT)
		{
			break;
		}
	}
	printf("\n");
	return 0;
}


static int uvc_set_fmt(int fd, CVI_PIXFMT_E pfmt, int w, int h, float frate)
{
	struct v4l2_format fmt;
	memset(&fmt, 0x0, sizeof(struct v4l2_format));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.pixelformat = pfmt;
	fmt.fmt.pix.height = h;
	fmt.fmt.pix.width = w;
	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	if(ioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
	{
		printf("set fmt failed with %s\n", strerror(errno));
		return -1;
	}

	if(ioctl(fd, VIDIOC_G_FMT, &fmt) == -1)
	{
		printf("get fmt failed with %s\n", strerror(errno));
		return -1;
	}

	printf("fmt.type:\t\t%d\n",fmt.type);
	printf("pix.pixelformat:\t%c%c%c%c\n",fmt.fmt.pix.pixelformat & 0xFF, (fmt.fmt.pix.pixelformat >> 8) & 0xFF,(fmt.fmt.pix.pixelformat >> 16) & 0xFF, (fmt.fmt.pix.pixelformat >> 24) & 0xFF);
	printf("pix.height:\t\t%d\n",fmt.fmt.pix.height);
	printf("pix.width:\t\t%d\n",fmt.fmt.pix.width);
	printf("pix.field:\t\t%d\n",fmt.fmt.pix.field);

	struct v4l2_streamparm param;
	memset(&param, 0x0, sizeof(struct v4l2_streamparm));
	param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	param.parm.capture.timeperframe.denominator = (__u32)frate;
	param.parm.capture.timeperframe.numerator = 1;

	if(ioctl(fd, VIDIOC_S_PARM, &param) == -1)
	{
		printf("set frate failed with %s\n", strerror(errno));
		return -1;
	}

	if(ioctl(fd, VIDIOC_G_PARM, &param) == -1)
	{
		printf("get frate failed with %s\n", strerror(errno));
		return -1;
	}
	printf("framerate:\t\t%d\n\n", param.parm.capture.timeperframe.denominator);

	return 0;
}

static int uvc_req_buffers(int fd, int num, struct v4l2_bufsinfo *bufs)
{
	struct v4l2_requestbuffers req;
	req.count = num;
	req.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory=V4L2_MEMORY_MMAP;
	if(ioctl(fd, VIDIOC_REQBUFS, &req) == -1)
	{
		printf("req buffer failed with %s\n", strerror(errno));
		return -1;
	}

	for(int i = 0; i < num; i++)
	{
		struct v4l2_buffer buf;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1)
		{
			printf("query buffer failed with %s\n", strerror(errno));
			return -1;
		}
		bufs[i].length = buf.length;
		bufs[i].start = mmap(NULL, bufs[i].length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
		if(bufs[i].start == MAP_FAILED)
		{
			printf("mmap %d len %d failed with %s\n", i, bufs[i].length, strerror(errno));
			return -1;
		}
	}
	printf("req buffers success\n\n");
	return 0;
}

static int uvc_stream_on(int fd, int num)
{
	for(int i = 0; i < num; i++)
	{
		struct v4l2_buffer buf;
		memset(&buf, 0x0, sizeof(struct v4l2_buffer));
		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
		ioctl(fd, VIDIOC_QBUF, &buf);
	}

	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	return ioctl(fd, VIDIOC_STREAMON, &type);
}

static int uvc_get_video_frame(int fd, int index)
{
	struct v4l2_buffer buf;
	memset(&buf, 0x0, sizeof(struct v4l2_buffer));
	buf.index = index;
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
	if(ioctl(fd, VIDIOC_DQBUF, &buf) == -1)
	{
		printf("get video failed with %s\n", strerror(errno));
		return -1;
	}
	return buf.bytesused;
}

static int uvc_release_video_frame(int fd, int index)
{
    struct v4l2_buffer buf;
    memset(&buf, 0x0, sizeof(struct v4l2_buffer));
    buf.index = index;
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    return ioctl(fd, VIDIOC_QBUF, &buf);
}

static int uvc_stream_off(int fd)
{
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	return ioctl(fd, VIDIOC_STREAMOFF, &type);
}

int cvi_uvc_req_filename(char *fname, int len, CVI_PIXFMT_E fmt)
{
#define PATH_PREFIX "./"
    if(len > 0)
    {
        tzset();
        time_t now;
	    struct tm t;
	    time(&now);
	    localtime_r(&now, &t);
	    snprintf(fname, len, PATH_PREFIX"/%04d%02d%02d_%02d%02d%02d.%s",
	        t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, (fmt == CVI_PIXFMT_YUYV_E) ? ("yuv") : ("jpg"));
    }
    return -1;
}

int cvi_uvc_start(void *ctx)
{
    CVI_UVC_HOST_CTX *c = (CVI_UVC_HOST_CTX *)ctx;
    if(c)
    {
        if(c->start_flag == 1)
            return 0;

        if(uvc_stream_on(c->fd, REQ_BUFS_CNT) != -1)
        {
            c->start_flag = 1;
            return 0;
        }
    }
    return -1;
}

int cvi_uvc_stop(void *ctx)
{
    CVI_UVC_HOST_CTX *c = (CVI_UVC_HOST_CTX *)ctx;
    if(c)
    {
        if(c->start_flag == 0)
            return 0;

        c->start_flag = 0;
        usleep(100 * 1000);
        return uvc_stream_off(c->fd);
    }
    return -1;
}


void cvi_uvc_destroy(void *ctx)
{
    CVI_UVC_HOST_CTX *c = (CVI_UVC_HOST_CTX *)ctx;
    if(c)
    {
#if CONTINOUS_DET_CAMERA
        if(c->video_tid)
        {
            c->video_exit = 1;
            pthread_join(c->video_tid, NULL);
        }
#endif
        if(c->start_flag == 1)
        {
            cvi_uvc_stop(ctx);
        }

        for(int i = 0; i < REQ_BUFS_CNT; i++)
		{
			if(c->bufs[i].start)
				munmap(c->bufs[i].start, c->bufs[i].length);
		}

        if(c->fd > 0)
            uvc_close(c->fd);

        free(c);
    }
}

void *cvi_uvc_create(const char *dev, CVI_PIXFMT_E pfmt, unsigned int w, unsigned int h, float frate)
{
    int i;
    int ret = 0;

    if(dev == NULL || strlen(dev) <= 0 || pfmt >= CVI_PIXFMT_BUTT || w <= 0 || h <= 0 || frate <= 0.001)
        goto FAILED;

    CVI_UVC_HOST_CTX *ctx = (CVI_UVC_HOST_CTX *)malloc(sizeof(CVI_UVC_HOST_CTX));
    if(ctx == NULL)
    {
        printf("cvi_uvc_create out of mem with %s\n", strerror(errno));
        goto FAILED;
    }

    memset(ctx, 0x0, sizeof(CVI_UVC_HOST_CTX));

    ctx->fd = uvc_open(dev);
    if(ctx->fd < 0)
    {
        printf("cvi_uvc_create open failed with %s\n", strerror(errno));
        goto FAILED;
    }

    ret = uvc_get_capability(ctx->fd);
    if(ret != 0xff)
    {
        printf("cvi_uvc_create dev %s is not a uvc\n", dev);
        goto FAILED;
    }

    struct v4l2_fmtdesc pfmtdesc[FMT_MAX_CNT];
    memset(pfmtdesc, 0x0, sizeof(struct v4l2_fmtdesc) * FMT_MAX_CNT);
    struct v4l2_frmsizeenum pframsize[FMT_MAX_CNT * 2];
    memset(pframsize, 0x0, sizeof(struct v4l2_frmsizeenum) * FMT_MAX_CNT * 2);
    ret = uvc_get_fmtdesc(ctx->fd, pfmtdesc, pframsize);
    if(ret != 0)
        goto FAILED;

    unsigned int u32Fmt = 0;
    if(pfmt == CVI_PIXFMT_YUYV_E)
        u32Fmt = V4L2_PIX_FMT_YUYV;
    else if(pfmt == CVI_PIXFMT_MJPEG_E)
        u32Fmt = V4L2_PIX_FMT_MJPEG;

    for(i = 0; i < FMT_MAX_CNT; i++)
    {
        if(pfmtdesc[i].pixelformat == u32Fmt)
            break;
    }

    if(i == FMT_MAX_CNT)
    {
        printf("cvi_uvc_create format not support\n");
        goto FAILED;
    }

    for(i = 0; i < FMT_MAX_CNT; i++)
    {
        if(pframsize[i].pixel_format == u32Fmt)
        {
            if(pframsize[i].type == V4L2_FRMSIZE_TYPE_DISCRETE)
            {
                if(pframsize[i].discrete.width == w && pframsize[i].discrete.height == h)
                {
                    break;
                }
            }
            else if (pframsize[i].type == V4L2_FRMSIZE_TYPE_STEPWISE)
            {

            }
        }
    }

    if(i == FMT_MAX_CNT * 2)
    {
        printf("cvi_uvc_create resolution not support\n");
        goto FAILED;
    }

    ret = uvc_set_fmt(ctx->fd, u32Fmt, w, h, frate);
    if(ret != 0)
    {
        printf("cvi_uvc_create set fmt failed\n");
        goto FAILED;
    }

    ret = uvc_req_buffers(ctx->fd, REQ_BUFS_CNT, ctx->bufs);
    if(ret != 0)
    {
        printf("cvi_uvc_create req bufs failed\n");
        goto FAILED;
    }

    ctx->fmt = pfmt;

    return ctx;

FAILED:
    cvi_uvc_destroy(ctx);
    return NULL;
}

// static void *cvi_uvc_thread(void *arg)
// {
//     CVI_UVC_HOST_CTX *c = (CVI_UVC_HOST_CTX *)arg;
//     if(c)
//     {
//         while(!c->video_exit)
//         {
//             for(int i = 0; i < REQ_BUFS_CNT; i++)
//             {
//                 if(c->start_flag == 1)
//                 {
//                     int len = uvc_get_video_frame(c->fd, i);
//                     if(len > 0)
//                     {
//                     #ifdef _OPEN_TEST_FUNC_
//                         if(c->testfd > 0)
//                         {
//                             write(c->testfd, c->bufs[i].start, len);
//                         }
//                     #endif
//                         uvc_release_video_frame(c->fd, i);
//                     }
//                 }
//             }
//             usleep(20 * 1000);
//         }
//     }
//     return NULL;
// }

struct uvc_hotplug{
    char devname[64];
    char action[16];
    int timeout_ms;
};

int cvi_uvc_snap(void *ctx)
{
	int ret = 0;
    CVI_UVC_HOST_CTX *c = (CVI_UVC_HOST_CTX *)ctx;
    if(c)
    {
        struct timespec tv;
        clock_gettime(CLOCK_MONOTONIC, &tv);
        for(int i = 0; i < REQ_BUFS_CNT; i++)
        {
            int len = uvc_get_video_frame(c->fd, i);
            if(len > 0)
            {
            #ifdef _OPEN_TEST_FUNC_
				cvi_uvc_req_filename(c->fname, sizeof(c->fname), c->fmt);
				c->testfd= open(c->fname, O_CREAT | O_RDWR);
				printf("create test file %s\n", c->fname);
                printf("snap file size %u\n", len);
                if(c->testfd > 0)
                {
					ret = write(c->testfd, c->bufs[i].start, len);
					if (ret < len)
						printf("wrire not filish!!!\n");
					close(c->testfd);
                }
                struct timespec tv1;
                clock_gettime(CLOCK_MONOTONIC, &tv1);
                int t1 = (tv1.tv_sec - tv.tv_sec) * 1000 + (tv1.tv_nsec - tv.tv_nsec) / 1000 / 1000;
                printf("[%s]: snap file take [%d] ms\n", c->fname, t1);
            #endif
                uvc_release_video_frame(c->fd, i);
                return 0;
            }
        }
    }
    return -1;
}

#if CONTINOUS_DET_CAMERA
static int uvc_listen_hotplug_event(void)
{
    struct sockaddr_nl snl;
    const int buffersize = EVENT_MSG_BUF_SIZE;
    int retval;

    memset(&snl, 0x00, sizeof(struct sockaddr_nl));
    snl.nl_family = AF_NETLINK;
    snl.nl_pid = getpid();
    snl.nl_groups = NETLINK_KOBJECT_UEVENT;

    int s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (s == -1)
    {
        printf("create socket failed with %s", strerror(errno));
        return -1;
    }

    /* set receive buffersize */
    setsockopt(s, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize));
    retval = bind(s, (struct sockaddr *) &snl, sizeof(struct sockaddr_nl));
    if (retval < 0) {
        printf("bind failed with %s", strerror(errno));
        close(s);
        s = -1;
        return -1;
    }
    return s;
}

static void *uvc_snap_thread(void *arg)
{
    struct uvc_hotplug *hplug = (struct uvc_hotplug *)arg;
    if(hplug == NULL)
        return NULL;

    CVI_UVC_HOST_CTX *ctx = NULL;
    ctx = (CVI_UVC_HOST_CTX *)cvi_uvc_create(hplug->devname, CVI_PIXFMT_MJPEG_E, 1920, 1080, 15.0);
    if(ctx == NULL)
        return NULL;

    cvi_uvc_start((void *)ctx);

    struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC, &tv);
    while(hplug->timeout_ms > 0)
    {
        for(int i = 0; i < REQ_BUFS_CNT; i++)
        {
            int len = uvc_get_video_frame(ctx->fd, i);
            if(len > 0)
                uvc_release_video_frame(ctx->fd, i);
        }
        struct timespec tv1;
        clock_gettime(CLOCK_MONOTONIC, &tv1);
        int t1 = (tv1.tv_sec - tv.tv_sec) * 1000 + (tv1.tv_nsec - tv.tv_nsec) / 1000 / 1000;
        if(t1 >= hplug->timeout_ms)
            break;
    }

    cvi_uvc_snap((void *)ctx);
    cvi_uvc_stop((void *)ctx);
    printf("uvc_snap_thread success %s\n", ctx->fname);
    cvi_uvc_destroy((void *)ctx);
    free(hplug);
    return NULL;
}

static int uvc_read_line(char *src, char *dst, int len)
{
    char *tmp = src;
    while(*tmp++ != '\0')
    {
        if(*tmp == '\n' || (*tmp =='\r' && *tmp++ == '\n'))
            break;
    }
    int l = tmp - src;
    strncpy(dst, src, (l <= len) ? l : (len));
    return l;
}

static int uvc_parse_hotplug_msg(char *msg, int len, struct uvc_hotplug *hplug)
{
#define STR_VIDEO_SEG0 "add@"
#define STR_VIDEO_SEG1 "remove@"
#define STR_VIDEO_SEG2 "video4linux/video"
    int cnt = 0;
    while(1)
    {
        char buf[512] = {0};
        int l = uvc_read_line(msg + cnt, buf, sizeof(buf) - 1);
        cnt += l;
        if(cnt >= len)
            break;

        if(strcasestr(buf, STR_VIDEO_SEG0) != NULL && strcasestr(buf, STR_VIDEO_SEG2) != NULL)
        {
            snprintf(hplug->devname, sizeof(hplug->devname), "/dev%s", strrchr(buf, '/'));
            snprintf(hplug->action, sizeof(hplug->action), "%s", "add");
            printf("add devname %s\n", hplug->devname);
        }
        else if(strcasestr(buf, STR_VIDEO_SEG1) != NULL && strcasestr(buf, STR_VIDEO_SEG2) != NULL)
        {
            snprintf(hplug->devname, sizeof(hplug->devname), "/dev%s", strrchr(buf, '/'));
            snprintf(hplug->action, sizeof(hplug->action), "%s", "remove");
            printf("remove devname %s\n", hplug->devname);
        }
    }
    return 0;
}

static void *cvi_uvc_hotplug_thread(void *arg)
{
    int event_sock = -1;
    int event_exit = 0;
    char *msg = (char *)malloc(EVENT_MSG_BUF_SIZE);

    do{
        event_sock = uvc_listen_hotplug_event();
        usleep(500 * 1000);
    }while(!event_exit && event_sock <= 0);

    while(!event_exit)
    {
        memset(msg, 0x0, EVENT_MSG_BUF_SIZE);
        int ret = recv(event_sock, msg, EVENT_MSG_BUF_SIZE - 1, 0);
        if(ret > 0)
        {
            struct uvc_hotplug *hplug = (struct uvc_hotplug *)malloc(sizeof(struct uvc_hotplug));
            uvc_parse_hotplug_msg(msg, ret, hplug);
            if(strcmp(hplug->action, "add") == 0)
            {
                hplug->timeout_ms = 2000;
                pthread_t tid;
                pthread_create(&tid, NULL, uvc_snap_thread, (void *)hplug);
                pthread_detach(tid);
            }
            else
            {
                free(hplug);
            }
        }
        usleep(20 * 1000);
    }
    if(msg)
        free(msg);
    return NULL;
}
#endif

int main(int argc, char **argv)
{

#if CONTINOUS_DET_CAMERA
    pthread_t event_tid;
    pthread_create(&event_tid, NULL, cvi_uvc_hotplug_thread, NULL);
    pthread_detach(event_tid);

    while(1)
    {
        sleep(1);
    }
#else
	if (argc < 2)
	{
		printf("please provide video dev node\n");
		return -1;
	}
#endif

    CVI_UVC_HOST_CTX *pst_uvc_ctx = NULL;
    printf("step 0: cvi_uvc_create start\n");
    pst_uvc_ctx = (CVI_UVC_HOST_CTX *)cvi_uvc_create(argv[1], CVI_PIXFMT_MJPEG_E, 640, 480, 25);
	printf("640*480\n");
    //pst_uvc_ctx = (CVI_UVC_HOST_CTX *)cvi_uvc_create(argv[1], CVI_PIXFMT_YUYV_E, 1280, 720, 15.0);
    if(pst_uvc_ctx == NULL)
        return -1;

    //printf("step 1: cvi_uvc_thread start\n");
    //pthread_create(&pst_uvc_ctx->video_tid, NULL, cvi_uvc_thread, (void *)pst_uvc_ctx);

    printf("step 2: cvi_uvc_start start\n");
    cvi_uvc_start((void *)pst_uvc_ctx);
    cvi_uvc_snap((void *)pst_uvc_ctx);

    printf("step 3: cvi_uvc_stop start\n");
    cvi_uvc_stop((void *)pst_uvc_ctx);

    printf("step 4: cvi_uvc_destroy start\n");
    cvi_uvc_destroy((void *)pst_uvc_ctx);
	return 0;
}
