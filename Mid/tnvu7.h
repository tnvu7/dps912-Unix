typedef struct peripheral_info {
    int num_channels;
    int size_channel;
} PERIPHERAL_INFO;

#define MIDTERM_GET_INFO          _IOR('b','b',PERIPHERAL_INFO*)//IOCTL get (read) command
#define MIDTERM_GET_CHANNEL_INDEX _IOR('a','b',int*)//IOCTL get (read) command
#define MIDTERM_SET_CHANNEL_INDEX _IOW('a','b',int*)