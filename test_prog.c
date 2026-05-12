#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define PROC_FILE_PATH "/proc/TEST_KURSOVA"
#define SYSFS_FILE_PATH "/sys/module/my_module_proc_sysfs/my_sysfs/my_sysfs_var"

int read_from_proc(){
    printf("[read_from_proc] виконую\n");
    char buf[16];

    int fd = open(PROC_FILE_PATH, O_RDONLY);
    if (fd < 0) {
        perror("[read_from_proc] відкриття файлу провалилось\n");
        return -1;
    }

    int read_status = read(fd, buf, sizeof(buf) - 1);
    if (read_status < 0) {
        perror("[read_from_proc] читання провалилось\n");
        close(fd);
        return read_status;
    } 

    buf[read_status] = '\0';
    printf("[read_from_proc] прочитано з buf: %s\n", buf);

    close(fd);
    return read_status;
}

int write_to_proc(){
    printf("[write_to_proc] виконую\n");
    const char *buf = "8bit0-0";

    int fd = open(PROC_FILE_PATH, O_WRONLY);
    if (fd < 0) {
        perror("[write_to_proc] відкриття файлу провалилось\n");
        return -1;
    }

    int bytes_writen = write(fd, buf, sizeof(buf)); 
    if (bytes_writen <= 0){
        perror("[write_to_proc] запис провалився\n");
        close(fd);
        return -1;
    }

    printf("[write_to_proc] записано %d байт\n", bytes_writen);    
    close(fd);
    return bytes_writen;
}

int read_from_sysfs(){
    printf("[read_from_sysfs] виконую\n");
    char buf[16];
    short read_var;

    int fd = open(SYSFS_FILE_PATH, O_RDONLY);
    if (fd < 0) {
        perror("[read_from_sysfs] відкриття файлу провалилось\n");
        return -1;
    }

    int read_status = read(fd, buf, sizeof(buf));
    if (read_status < 0) {
        perror("[read_from_sysfs] читання провалилось\n");
        close(fd);
        return read_status;
    } 

    buf[read_status] = '\0';
    
    scanf(buf, "%hd", &read_var);

    printf("[read_from_sysfs] прочитано - %hd\n", read_var);

    close(fd);
    return read_status;
}

int write_to_sysfs(short var){
    printf("[write_to_sysfs] виконую\n");
    char buf[16];
    int txt_len = snprintf(buf, sizeof(buf), "%hd", var);


    int fd = open(SYSFS_FILE_PATH, O_WRONLY);
    if (fd < 0) {
        perror("[write_to_sysfs] відкриття файлу провалилось\n");
        return -1;
    }

    int bytes_writen = write(fd, buf, txt_len); 
    if (bytes_writen <= 0){
        perror("[write_to_sysfs] запис провалився\n");
        close(fd);
        return -1;
    }

    printf("[write_to_sysfs] записано %d байт\n", bytes_writen);    
    close(fd);
    return bytes_writen;
}

int main(int argc, char const *argv[]){
    printf("Test_prog [main]\n");
    
    read_from_proc();

    write_to_sysfs(78);

    read_from_sysfs();
    return 0;
}


