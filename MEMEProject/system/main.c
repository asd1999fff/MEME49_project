#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define DEVICE_DHT22 "/dev/dht22"
#define DEVICE_HW508 "/dev/hw508"
#define DEVICE_WHITELED "/dev/whiteLed"
#define DEVICE_BLTEST "/dev/MQ7"
#define SHM_KEY 4949 //Share Memorry KEY
#define ARRAY_SIZE 8


unsigned char tempz;
unsigned char coVal;
unsigned char *shm_flag;
static int counter_dht22 = 0;
static int counter_MQ7 = 0;
static int buzzer_fd = -1;  // Initial not open
static int whiteLed_fd = -1;  // Initial not open

unsigned long get_current_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;  // Returns the current time in milliseconds
}

// DHT22 OPEN
int dht22(void) {
    int temperature_fd;
    int ret = 0;
    char buf[5];
    unsigned char tempx = 0;

    temperature_fd = open(DEVICE_DHT22, O_RDONLY);
    if (temperature_fd < 0) {
        perror("can not open device");
        exit(1);
    }

    ret = read(temperature_fd, buf, sizeof(buf));
    if (ret < 0) {
        printf("read err!\n");
    } else {
        tempz = buf[2];
        tempx = buf[3];
        printf("temperature! = %d.%d\n", tempz, tempx);
    }

    close(temperature_fd);
    return tempz;
}

int MQ7() {
    int testfd;
	int ret = 0;
	unsigned char buf[2];
		
	testfd = open(DEVICE_BLTEST, O_RDONLY);
	if(testfd < 0)
	{
		perror("can not open device");
		exit(1);
	}
		ret = read(testfd,buf,sizeof(buf));
        if(ret < 0)
        {
            printf("read err!\n");
		}
        else
		{
            coVal = buf[1];
            printf("CO1! = %d\n", coVal);
		}
	if (testfd >= 0)	 //close humidityfd if open
	{
		close(testfd);
	}
	
	return coVal;
}


// Buzzer ON
void hw508_on(void) {
    if (buzzer_fd < 0) {
        buzzer_fd = open(DEVICE_HW508, O_WRONLY);
        if (buzzer_fd < 0) {
            perror("can not open device");
            exit(1);
        }
    }

    if (write(buzzer_fd, "1", 1) < 0) {
        perror("write failed");
        close(buzzer_fd);
        buzzer_fd = -1;
        exit(1);
    }
}

// Buzzer OFF
void hw508_off(void) {
    if (buzzer_fd >= 0) {
        if (write(buzzer_fd, "0", 1) < 0) {
            perror("write failed");
            close(buzzer_fd);
            buzzer_fd = -1;
            exit(1);
        }
        close(buzzer_fd);
        buzzer_fd = -1;  // not open
    }
}

// OPEN ON
void whiteLed_on(void) {
    if (whiteLed_fd < 0) {
        whiteLed_fd = open(DEVICE_WHITELED, O_WRONLY);
        if (whiteLed_fd < 0) {
            perror("can not open device");
            exit(1);
        }
    }

    if (write(whiteLed_fd, "1", 1) < 0) {
        perror("write failed");
        close(whiteLed_fd);
        whiteLed_fd = -1;
        exit(1);
    }
}

// LED OFF
void whiteLed_off(void) {
    if (whiteLed_fd >= 0) {
        if (write(whiteLed_fd, "0", 1) < 0) {
            perror("write failed");
            close(whiteLed_fd);
            whiteLed_fd = -1;
            exit(1);
        }
        close(whiteLed_fd);
        whiteLed_fd = -1;  // not open
    }
}


// interrupt
void handle_signal(int signal) {
    if (signal == SIGINT) {
        hw508_off();
        whiteLed_off();
        exit(0); 
    }
}

int main() {
    unsigned long last_time_dht22 = 0;
    unsigned long last_time_dht22_std = 0;
    unsigned long last_time_hw508 = 0;
    unsigned long last_time_MQ7 = 0;
    unsigned long last_time_MQ7_std = 0;
    unsigned long current_time;
    int buzzer_flag = 0;


    //Shared Memory set
    int shmid = shmget(SHM_KEY, ARRAY_SIZE * sizeof(int), 0644 | IPC_CREAT);
    if (shmid == -1) {
        printf("shmget failed1");
        exit(1);
    }

    //Shared Memory put
    shm_flag = (unsigned char *)shmat(shmid, NULL, 0);
    if (shm_flag == (unsigned char *)(-1)) {
        printf("shmat failed2\n");
            exit(1);
    }

    signal(SIGINT, handle_signal);

    while (1) {
        current_time = get_current_time();

        if (current_time - last_time_dht22 >= 2000) {  // 2s
            dht22();
            shm_flag[0] = tempz;
            last_time_dht22 = current_time;
        }

        if (current_time - last_time_MQ7 >= 2000) {  // 2s
            MQ7();
            shm_flag[1] = coVal;
            last_time_MQ7 = current_time;
        }

        //Meet the standard
        if (tempz > 45 && coVal > 120 || counter_dht22 > 30 || counter_MQ7 > 30) {
            shm_flag[2] = 1;
            if (current_time - last_time_hw508 >= 250) {  // Buzzer frequency 250 ms
                if (buzzer_flag) {
                    hw508_off();  // Buzzer OFF
                } else {
                    hw508_on();   // Buzzer ON
                }
                buzzer_flag = !buzzer_flag; 
                last_time_hw508 = current_time;
            }

            // OPEN LED 
            if (whiteLed_fd < 0) {
                whiteLed_on();
            }

        } else {
            shm_flag[2] = 0;
            // If the temperature < 20 ; CO < 60, turn off the LED & Buzzer
            if (buzzer_flag) {
                hw508_off();
                buzzer_flag = 0;
            }
            if (whiteLed_fd >= 0) {
                whiteLed_off();
            }
        }

        if (current_time - last_time_dht22_std >= 1000){  //1s
            if (tempz > 45) {
                counter_dht22++;
                // printf("counter_dht22 = %d\n", counter_dht22);
                last_time_dht22_std = current_time;
            }    
            else {
                counter_dht22 = 0;
                // printf("counter_dht22 = %d\n", counter_dht22);
                last_time_dht22_std = current_time;
            }
        }

        if (current_time - last_time_MQ7_std >= 1000){  //1s
            if (coVal > 120) {
                counter_MQ7++;
                // printf("counter_MQ7 = %d\n", counter_MQ7);
                last_time_MQ7_std = current_time;
            }    
            else {
                counter_MQ7 = 0;
                // printf("counter_MQ7 = %d\n", counter_MQ7);
                last_time_MQ7_std = current_time;
            }
        }
        usleep(100000);
    }

    return 0;
}