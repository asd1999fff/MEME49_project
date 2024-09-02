#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHM_KEY 4949
#define ARRAY_SIZE 8

unsigned char tempz;
unsigned char coVal;
unsigned char *shm_flag;

void camera_capture() {
    int status;
    status = system("ffmpeg -f video4linux2 -i /dev/video0 -s 640x480 -vframes 1 -loglevel quiet -y ../excute/node_js/public/alert_image.jpg");
    if (status == -1){
        printf("Camera failed !!!");
    }
}

int dht22_write(void) {
    FILE *file;
    file = fopen("../excute/node_js/Data/temp_output.csv", "w");
    if(file == NULL)
    {
        printf("Can't Find status.txt\n");
    }
	fprintf(file, "number\n");
	fprintf(file, "%d\n", tempz);
	fclose(file);
    return 0;
}

int MQ7_write(void) {
    FILE *file;
    file = fopen("../excute/node_js/Data/CO1_output.csv", "w");
    if(file == NULL)
    {
        printf("Can't Find status.txt\n");
    }
		fprintf(file, "number\n");
		fprintf(file, "%d\n", coVal);
		fclose(file);
    return 0;
}

int camera_write(int a) {
    FILE *file;
    file = fopen("../excute/node_js/Data/camera_output.csv", "w");
    if(file == NULL)
    {
        printf("Can't Find status.txt\n");
    }
		fprintf(file, "camera\n");
		fprintf(file, "%d\n", a);
		fclose(file);
    return 0;
}


int main() {
    //Shared Memory set
    int shmid = shmget(SHM_KEY, ARRAY_SIZE * sizeof(int), 0644 );
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    //Shared Memory get
    shm_flag = (unsigned char *)shmat(shmid, NULL, 0);
    if (shm_flag == (unsigned char *)(-1)) {
        perror("shmat failed\n");
            exit(1);
    }

    while (1) {
        tempz = shm_flag[0];
        coVal = shm_flag[1];
        dht22_write();
        MQ7_write();
        if (shm_flag[2]) {  // temperature > 45 & coVal > 60
            camera_capture();
            printf("Camera capture! flag = %d\n", shm_flag[2]);
            camera_write(1);
            sleep(1);  // 1s 
        }else{
            printf("Camera failed! flag = %d\n", shm_flag[2]);
            camera_write(0);
            sleep(1);  // 1s 
        }
    }

    return 0;
}