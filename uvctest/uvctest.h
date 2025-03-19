#ifndef _UVC_TEST_H_
#define _UVC_TEST_H_

#include <sys/time.h>

/*
 * Common stuff for both V4L1 and V4L2
 * Moved from videodev.h
 */
#define VIDEO_MAX_FRAME               32
#define VIDEO_MAX_PLANES               8

/*
 *	M I S C E L L A N E O U S
 */

/*  Four-character-code (FOURCC) */
#define v4l2_fourcc(a, b, c, d)\
	((__u32)(a) | ((__u32)(b) << 8) | ((__u32)(c) << 16) | ((__u32)(d) << 24))
#define v4l2_fourcc_be(a, b, c, d)	(v4l2_fourcc(a, b, c, d) | (1 << 31))


typedef unsigned char __u8;
typedef unsigned short __u16;
typedef unsigned int  __u32;
typedef int  __s32;

struct v4l2_capability {
	__u8	driver[16];
	__u8	card[32];
	__u8	bus_info[32];
	__u32   version;
	__u32	capabilities;
	__u32	device_caps;
	__u32	reserved[3];
};


/*
 *	F O R M A T   E N U M E R A T I O N
 */
struct v4l2_fmtdesc {
	__u32		    index;             /* Format number      */
	__u32		    type;              /* enum v4l2_buf_type */
	__u32               flags;
	__u8		    description[32];   /* Description string */
	__u32		    pixelformat;       /* Format fourcc      */
	__u32		    reserved[4];
};


/*
 *	E N U M S
 */
enum v4l2_field {
	V4L2_FIELD_ANY           = 0, /* driver can choose from none,
					 top, bottom, interlaced
					 depending on whatever it thinks
					 is approximate ... */
	V4L2_FIELD_NONE          = 1, /* this device has no fields ... */
	V4L2_FIELD_TOP           = 2, /* top field only */
	V4L2_FIELD_BOTTOM        = 3, /* bottom field only */
	V4L2_FIELD_INTERLACED    = 4, /* both fields interlaced */
	V4L2_FIELD_SEQ_TB        = 5, /* both fields sequential into one
					 buffer, top-bottom order */
	V4L2_FIELD_SEQ_BT        = 6, /* same as above + bottom-top order */
	V4L2_FIELD_ALTERNATE     = 7, /* both fields alternating into
					 separate buffers */
	V4L2_FIELD_INTERLACED_TB = 8, /* both fields interlaced, top field
					 first and the top field is
					 transmitted first */
	V4L2_FIELD_INTERLACED_BT = 9, /* both fields interlaced, top field
					 first and the bottom field is
					 transmitted first */
};

enum v4l2_buf_type {
	V4L2_BUF_TYPE_VIDEO_CAPTURE        = 1,
	V4L2_BUF_TYPE_VIDEO_OUTPUT         = 2,
	V4L2_BUF_TYPE_VIDEO_OVERLAY        = 3,
	V4L2_BUF_TYPE_VBI_CAPTURE          = 4,
	V4L2_BUF_TYPE_VBI_OUTPUT           = 5,
	V4L2_BUF_TYPE_SLICED_VBI_CAPTURE   = 6,
	V4L2_BUF_TYPE_SLICED_VBI_OUTPUT    = 7,
	V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY = 8,
	V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE = 9,
	V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE  = 10,
	V4L2_BUF_TYPE_SDR_CAPTURE          = 11,
	V4L2_BUF_TYPE_SDR_OUTPUT           = 12,
	V4L2_BUF_TYPE_META_CAPTURE         = 13,
	/* Deprecated, do not use */
	V4L2_BUF_TYPE_PRIVATE              = 0x80,
};


/*
 *	V I D E O   I M A G E   F O R M A T
 */
struct v4l2_pix_format {
	__u32			width;
	__u32			height;
	__u32			pixelformat;
	__u32			field;		/* enum v4l2_field */
	__u32			bytesperline;	/* for padding, zero if unused */
	__u32			sizeimage;
	__u32			colorspace;	/* enum v4l2_colorspace */
	__u32			priv;		/* private data, depends on pixelformat */
	__u32			flags;		/* format flags (V4L2_PIX_FMT_FLAG_*) */
	union {
		/* enum v4l2_ycbcr_encoding */
		__u32			ycbcr_enc;
		/* enum v4l2_hsv_encoding */
		__u32			hsv_enc;
	};
	__u32			quantization;	/* enum v4l2_quantization */
	__u32			xfer_func;	/* enum v4l2_xfer_func */
};


/*
 *	A G G R E G A T E   S T R U C T U R E S
 */

/**
 * struct v4l2_plane_pix_format - additional, per-plane format definition
 * @sizeimage:		maximum size in bytes required for data, for which
 *			this plane will be used
 * @bytesperline:	distance in bytes between the leftmost pixels in two
 *			adjacent lines
 */
struct v4l2_plane_pix_format {
	__u32		sizeimage;
	__u32		bytesperline;
	__u16		reserved[6];
} __attribute__ ((packed));

/**
 * struct v4l2_pix_format_mplane - multiplanar format definition
 * @width:		image width in pixels
 * @height:		image height in pixels
 * @pixelformat:	little endian four character code (fourcc)
 * @field:		enum v4l2_field; field order (for interlaced video)
 * @colorspace:		enum v4l2_colorspace; supplemental to pixelformat
 * @plane_fmt:		per-plane information
 * @num_planes:		number of planes for this format
 * @flags:		format flags (V4L2_PIX_FMT_FLAG_*)
 * @ycbcr_enc:		enum v4l2_ycbcr_encoding, Y'CbCr encoding
 * @quantization:	enum v4l2_quantization, colorspace quantization
 * @xfer_func:		enum v4l2_xfer_func, colorspace transfer function
 */
struct v4l2_pix_format_mplane {
	__u32				width;
	__u32				height;
	__u32				pixelformat;
	__u32				field;
	__u32				colorspace;

	struct v4l2_plane_pix_format	plane_fmt[VIDEO_MAX_PLANES];
	__u8				num_planes;
	__u8				flags;
	 union {
		__u8				ycbcr_enc;
		__u8				hsv_enc;
	};
	__u8				quantization;
	__u8				xfer_func;
	__u8				reserved[7];
} __attribute__ ((packed));


struct v4l2_rect {
	__s32   left;
	__s32   top;
	__u32   width;
	__u32   height;
};

struct v4l2_fract {
	__u32   numerator;
	__u32   denominator;
};

struct v4l2_clip {
	struct v4l2_rect        c;
	struct v4l2_clip *next;
};

struct v4l2_window {
	struct v4l2_rect        w;
	__u32			field;	 /* enum v4l2_field */
	__u32			chromakey;
	struct v4l2_clip *clips;
	__u32			clipcount;
	void			*bitmap;
	__u8                    global_alpha;
};

/* Raw VBI */
struct v4l2_vbi_format {
	__u32	sampling_rate;		/* in 1 Hz */
	__u32	offset;
	__u32	samples_per_line;
	__u32	sample_format;		/* V4L2_PIX_FMT_* */
	__s32	start[2];
	__u32	count[2];
	__u32	flags;			/* V4L2_VBI_* */
	__u32	reserved[2];		/* must be zero */
};

struct v4l2_sliced_vbi_format {
	__u16   service_set;
	/* service_lines[0][...] specifies lines 0-23 (1-23 used) of the first field
	   service_lines[1][...] specifies lines 0-23 (1-23 used) of the second field
				 (equals frame lines 313-336 for 625 line video
				  standards, 263-286 for 525 line standards) */
	__u16   service_lines[2][24];
	__u32   io_size;
	__u32   reserved[2];            /* must be zero */
};

/**
 * struct v4l2_sdr_format - SDR format definition
 * @pixelformat:	little endian four character code (fourcc)
 * @buffersize:		maximum size in bytes required for data
 */
struct v4l2_sdr_format {
	__u32				pixelformat;
	__u32				buffersize;
	__u8				reserved[24];
} __attribute__ ((packed));

/**
 * struct v4l2_meta_format - metadata format definition
 * @dataformat:		little endian four character code (fourcc)
 * @buffersize:		maximum size in bytes required for data
 */
struct v4l2_meta_format {
	__u32				dataformat;
	__u32				buffersize;
} __attribute__ ((packed));

struct v4l2_format {
	__u32	 type;
	union {
		struct v4l2_pix_format		pix;     /* V4L2_BUF_TYPE_VIDEO_CAPTURE */
		struct v4l2_pix_format_mplane	pix_mp;  /* V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE */
		struct v4l2_window		win;     /* V4L2_BUF_TYPE_VIDEO_OVERLAY */
		struct v4l2_vbi_format		vbi;     /* V4L2_BUF_TYPE_VBI_CAPTURE */
		struct v4l2_sliced_vbi_format	sliced;  /* V4L2_BUF_TYPE_SLICED_VBI_CAPTURE */
		struct v4l2_sdr_format		sdr;     /* V4L2_BUF_TYPE_SDR_CAPTURE */
		struct v4l2_meta_format		meta;    /* V4L2_BUF_TYPE_META_CAPTURE */
		__u8	raw_data[200];                   /* user-defined */
	} fmt;
};


/*
 *	C A P T U R E   P A R A M E T E R S
 */
struct v4l2_captureparm {
	__u32		   capability;	  /*  Supported modes */
	__u32		   capturemode;	  /*  Current mode */
	struct v4l2_fract  timeperframe;  /*  Time per frame in seconds */
	__u32		   extendedmode;  /*  Driver-specific extensions */
	__u32              readbuffers;   /*  # of buffers for read */
	__u32		   reserved[4];
};

struct v4l2_outputparm {
	__u32		   capability;	 /*  Supported modes */
	__u32		   outputmode;	 /*  Current mode */
	struct v4l2_fract  timeperframe; /*  Time per frame in seconds */
	__u32		   extendedmode; /*  Driver-specific extensions */
	__u32              writebuffers; /*  # of buffers for write */
	__u32		   reserved[4];
};

/*	Stream type-dependent parameters
 */
struct v4l2_streamparm {
	__u32	 type;			/* enum v4l2_buf_type */
	union {
		struct v4l2_captureparm	capture;
		struct v4l2_outputparm	output;
		__u8	raw_data[200];  /* user-defined */
	} parm;
};

/*
 *	M E M O R Y - M A P P I N G   B U F F E R S
 */
struct v4l2_requestbuffers {
	__u32			count;
	__u32			type;		/* enum v4l2_buf_type */
	__u32			memory;		/* enum v4l2_memory */
	__u32			reserved[2];
};

struct v4l2_timecode {
	__u32	type;
	__u32	flags;
	__u8	frames;
	__u8	seconds;
	__u8	minutes;
	__u8	hours;
	__u8	userbits[4];
};



struct v4l2_buffer {
	__u32			index;
	__u32			type;
	__u32			bytesused;
	__u32			flags;
	__u32			field;
	struct timeval		timestamp;
	struct v4l2_timecode	timecode;
	__u32			sequence;

	/* memory location */
	__u32			memory;
	union {
		__u32           offset;
		unsigned long   userptr;
		struct v4l2_plane *planes;
		__s32		fd;
	} m;
	__u32			length;
	__u32			reserved2;
	__u32			reserved;
};

enum v4l2_memory {
	V4L2_MEMORY_MMAP             = 1,
	V4L2_MEMORY_USERPTR          = 2,
	V4L2_MEMORY_OVERLAY          = 3,
	V4L2_MEMORY_DMABUF           = 4,
};

enum v4l2_frmsizetypes {
	V4L2_FRMSIZE_TYPE_DISCRETE	= 1,
	V4L2_FRMSIZE_TYPE_CONTINUOUS	= 2,
	V4L2_FRMSIZE_TYPE_STEPWISE	= 3,
};

struct v4l2_frmsize_discrete {
	__u32			width;		/* Frame width [pixel] */
	__u32			height;		/* Frame height [pixel] */
};

struct v4l2_frmsize_stepwise {
	__u32			min_width;	/* Minimum frame width [pixel] */
	__u32			max_width;	/* Maximum frame width [pixel] */
	__u32			step_width;	/* Frame width step size [pixel] */
	__u32			min_height;	/* Minimum frame height [pixel] */
	__u32			max_height;	/* Maximum frame height [pixel] */
	__u32			step_height;	/* Frame height step size [pixel] */
};

struct v4l2_frmsizeenum {
	__u32			index;		/* Frame size number */
	__u32			pixel_format;	/* Pixel format */
	__u32			type;		/* Frame size type the device supports. */

	union {					/* Frame size */
		struct v4l2_frmsize_discrete	discrete;
		struct v4l2_frmsize_stepwise	stepwise;
	};

	__u32   reserved[2];			/* Reserved space for future use */
};

/* Luminance+Chrominance formats */
#define V4L2_PIX_FMT_YUYV    v4l2_fourcc('Y', 'U', 'Y', 'V') /* 16  YUV 4:2:2     */
#define V4L2_PIX_FMT_YYUV    v4l2_fourcc('Y', 'Y', 'U', 'V') /* 16  YUV 4:2:2     */

/* compressed formats */
#define V4L2_PIX_FMT_MJPEG    v4l2_fourcc('M', 'J', 'P', 'G') /* Motion-JPEG   */


/* Values for 'capabilities' field */
#define V4L2_CAP_VIDEO_CAPTURE		0x00000001  /* Is a video capture device */

#define V4L2_CAP_EXT_PIX_FORMAT		0x00200000  /* Supports the extended pixel format */

#define V4L2_CAP_STREAMING          0x04000000  /* streaming I/O ioctls */



#define VIDIOC_QUERYCAP		    _IOR('V',  0, struct v4l2_capability)
#define VIDIOC_ENUM_FMT         _IOWR('V',  2, struct v4l2_fmtdesc)
#define VIDIOC_G_FMT            _IOWR('V',  4, struct v4l2_format)
#define VIDIOC_S_FMT            _IOWR('V',  5, struct v4l2_format)
#define VIDIOC_REQBUFS		    _IOWR('V',  8, struct v4l2_requestbuffers)
#define VIDIOC_QUERYBUF		    _IOWR('V',  9, struct v4l2_buffer)
#define VIDIOC_QBUF	            _IOWR('V', 15, struct v4l2_buffer)
#define VIDIOC_DQBUF		    _IOWR('V', 17, struct v4l2_buffer)
#define VIDIOC_STREAMON		    _IOW('V', 18, int)
#define VIDIOC_STREAMOFF	    _IOW('V', 19, int)
#define VIDIOC_G_PARM		    _IOWR('V', 21, struct v4l2_streamparm)
#define VIDIOC_S_PARM		    _IOWR('V', 22, struct v4l2_streamparm)
#define VIDIOC_ENUM_FRAMESIZES	_IOWR('V', 74, struct v4l2_frmsizeenum)








#endif
