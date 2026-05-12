#define DEBUG
#define pr_fmt(fmt) "smod: " fmt

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

#define INTERNAL_BUFFER_SIZE 8

// Структура для proc
static struct proc_dir_entry *my_proc_dir_entry;
// Струкутра для kobject(sysfs)
static struct kobject *my_kobj;

// Зміни куди будуть підставлені параметри з insmod
static long int_param = 10;

static char internal_text_buffer_file[INTERNAL_BUFFER_SIZE] = "Mod txt\n";

static unsigned short my_sysfs_var = 13;

// ФУНКЦІЇ
static void __init desc_text_for_init(void);
static void show_mparam_var(void);

// Функції які приймають парметри і пихають у змінні
// 0644 root(R/W), інші(R only)
module_param(int_param, long, 0644); 
MODULE_PARM_DESC(int_param, "Параметр для передачі аргументів\n");
module_param_string(msg, internal_text_buffer_file, sizeof(internal_text_buffer_file), 0644);
MODULE_PARM_DESC(internal_text_buffer_file, "Масив для запису String згідно варіанту 5. 16 byte\n");

// Функції proc
static ssize_t my_proc_read(struct file *fptr, char __user *buffer, size_t buff_len, loff_t *offset);
static ssize_t my_proc_write(struct file *fptr, const char __user *buffer, size_t buff_len, loff_t *offset);

// Функції sysfs
static ssize_t my_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf); 
static ssize_t my_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count);

// Структури proc
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

// Структури sysfs
static struct kobj_attribute mykobj_atr = __ATTR(my_sysfs_var, 0644, my_show, my_store);

static void __init desc_text_for_init(void){
    pr_info("Курсова модуль ядра варіант 5 Паничшин\n");
}

static void show_mparam_var(void){
    pr_info("Змінна int_param - %d\n", int_param);
    pr_info("Змінна internal_text_buffer_file: %s\n", internal_text_buffer_file);
    pr_info("Змінна my_sysfs_var - %d\n", my_sysfs_var);
}

static ssize_t my_proc_read(struct file *fptr, char __user *usr_buffer, size_t usr_buff_len, loff_t *buf_pos){
    pr_info("Виконується [my_proc_read] це proc\n");
    int module_buff_length = sizeof(internal_text_buffer_file);
    ssize_t returnCTURes = module_buff_length;

    if (*buf_pos >= module_buff_length) {
        pr_info("[my_proc_read] Читання вже завершене, ми на ласт символі\n"); 
        return 0;
    }
    // Віддаємо user рівно стільки байт скільки у нас є в буфері. Якщо віддати більше, вийдемо за межі 
    // і почнемо передавати сміття == kernel panic
    if (usr_buff_len > module_buff_length) usr_buff_len = module_buff_length - *buf_pos;

    returnCTURes = copy_to_user(usr_buffer, internal_text_buffer_file + *buf_pos, usr_buff_len);
    
    if (returnCTURes != 0) {
    pr_info("[my_proc_read] Помилка копіювання\n"); 
    return -EFAULT;
    }

    // Щоб повторне читання відбулось з місце де закнічили
    *buf_pos += usr_buff_len;
    pr_info("[my_proc_read] дійшов до <return usr_buff_len>\n");

    return usr_buff_len;
}

static ssize_t my_proc_write(struct file *fptr, const char __user *usr_buffer, size_t num_to_write, loff_t *buf_pos){
    pr_info("Виконується [my_proc_write] це proc\n");
    int max_len = INTERNAL_BUFFER_SIZE - 1;

    if (num_to_write > max_len) num_to_write = max_len;

    if (copy_from_user(internal_text_buffer_file, usr_buffer, num_to_write)){ 
        pr_info("[my_proc_write] Операція запису провалилась\n");
        return -EFAULT;
    }

    if (num_to_write > 0 && internal_text_buffer_file[num_to_write-1] == '\n'){
        internal_text_buffer_file[num_to_write - 1] = '\0';
    } else {
        internal_text_buffer_file[num_to_write] = '\0';
    }

    *buf_pos += num_to_write;
    
    pr_info("[my_proc_write]Операція запису завершилась успішно\n");
    return num_to_write;
}

static ssize_t my_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    pr_info("Виконується [my_show] це sysfs\n");
    return sprintf(buf, "%hd\n", my_sysfs_var);
}

static ssize_t my_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
    pr_info("Виконується [my_store] це sysfs\n");
    sscanf(buf, "%d\n", &my_sysfs_var);
    return count;
}


static int __init proc_sysfs_module_init(void){
    pr_info("[proc_sysfs_module_init] воконується\n");
    desc_text_for_init();
    show_mparam_var();

    my_proc_dir_entry = proc_create("TEST_KURSOVA", 0644, NULL, &proc_file_fops);
    pr_info("[proc_sysfs_module_init] proc_create() виконано\n");

    my_kobj = kobject_create_and_add("my_sysfs", &THIS_MODULE->mkobj.kobj);
    sysfs_create_file(my_kobj, &mykobj_atr.attr);
    pr_info("[proc_sysfs_module_init] sysfs_create_file() виконано\n");

    return 0;
}

static void __exit proc_sysfs_module_exit(void){
    pr_info("[proc_sysfs_module_exit] My proc видално з пам'яті\n");
    proc_remove(my_proc_dir_entry);

    kobject_put(my_kobj);
}

module_init(proc_sysfs_module_init);
module_exit(proc_sysfs_module_exit);

MODULE_LICENSE("GPL");