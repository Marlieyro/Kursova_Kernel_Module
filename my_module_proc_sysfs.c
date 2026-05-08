#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/uaccess.h>
#include <linux/moduleparam.h>
#include <linux/stat.h> // Для параметрів доступу у функції module_param
#include <linux/proc_fs.h> 

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif

// #define pr_fmt(fmt) "smod: " fmt
#define INTERNAL_BUFFER_SIZE 256

// Структура для proc
static struct proc_dir_entry *my_proc_dir_entry;

// Зміни куди будуть підставлені параметри з insmod
static int int_param = 10;
static char internal_text_buffer_file[INTERNAL_BUFFER_SIZE] = "Текст з модуля(файлу), це перший текст.\n";

// Функції які приймають парметри і пихають у змінні
// 0644 root(R/W), інші(R only)
module_param(int_param, int, 0644); 
MODULE_PARM_DESC(int_param, "Тестовий параметр для передачі аргументів\n");
module_param_string(msg, internal_text_buffer_file, sizeof(internal_text_buffer_file), 0644);
MODULE_PARM_DESC(internal_text_buffer_file, "Масив для запису String згідно варіанту 5. 16 byte\n");


// Функції 
static void __init desc_text_for_init(void);
static void show_mparam_var(void);
static ssize_t my_proc_read(struct file *fptr, char __user *buffer, size_t buff_len, loff_t *offset);
static ssize_t my_proc_write(struct file *fptr, const char __user *buffer, size_t buff_len, loff_t *offset);


static void __init desc_text_for_init(void){
    pr_info("Курсова модуль ядра варіант 5 Паничшин\n");
    pr_debug("Des was printed\n");   
}

static void show_mparam_var(void){
    pr_info("Тест int значення: %d\n", int_param);
    pr_info("String який у масиві [256s]: %s\n", internal_text_buffer_file);
}


static ssize_t my_proc_read(struct file *fptr, char __user *usr_buffer, size_t usr_buff_len, loff_t *buf_pos){
    int module_buff_length = sizeof(internal_text_buffer_file);
    ssize_t returnCTURes = module_buff_length;

    if (*buf_pos >= module_buff_length) {
        pr_debug("Читання вже завершене, ми на ласт символі\n"); 
        return 0;
    }
    // Віддаємо user рівно стільки байт скільки у нас є в буфері. Якщо віддати більше, вийдемо за межі 
    // і почнемо передавати сміття == kernel panic
    if (usr_buff_len > module_buff_length) usr_buff_len = module_buff_length - *buf_pos;

    returnCTURes = copy_to_user(usr_buffer, internal_text_buffer_file + *buf_pos, usr_buff_len);
    
    if (returnCTURes != 0) {
    pr_debug("Помилка копіювання\n"); 
    return -EFAULT;
    }

    // Щоб повторне читання відбулось з місце де закнічили
    *buf_pos += usr_buff_len;
    pr_debug("read() дійшов до <return usr_buff_len>\n");

    return usr_buff_len;

}

static ssize_t my_proc_write(struct file *fptr, const char __user *usr_buffer, size_t num_to_write, loff_t *buf_pos){
    int max_len = INTERNAL_BUFFER_SIZE - 1;

    if (num_to_write > max_len) num_to_write = max_len;

    if (copy_from_user(internal_text_buffer_file, usr_buffer, num_to_write)){ 
        pr_debug("Операція запису провалилась\n");
        return -EFAULT;
    }

    if (num_to_write > 0 && internal_text_buffer_file[num_to_write-1] == '\n'){
        internal_text_buffer_file[num_to_write - 1] = '\0';
    } else {
        internal_text_buffer_file[num_to_write] = '\0';
    }

    *buf_pos += num_to_write;
    
    pr_debug("Операція запису завершилась успішно\n");
    return num_to_write;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_file_fops = {
    .proc_read = my_proc_read,
    .proc_write = my_proc_write,
};
#else
static const struct file_operations proc_file_fops = {
    .read = proc_read,
    .write = proc_write,
};
#endif

static int __init proc_sysfs_module_init(void){
    desc_text_for_init();
    show_mparam_var();

    my_proc_dir_entry = proc_create("Kursova_Module", 0644, NULL, &proc_file_fops);
    pr_debug("proc_create() виконано");

    return 0;
}

static void __exit proc_sysfs_module_exit(void){
    proc_remove(my_proc_dir_entry);\
    pr_info("My proc видално з пам'яті");
}

module_init(proc_sysfs_module_init);
module_exit(proc_sysfs_module_exit);

MODULE_LICENSE("GPL");