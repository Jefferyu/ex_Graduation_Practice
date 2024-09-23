#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <gdk/gdkx.h>

// 定义mplayer命令和管道文件
#define MPLAYER_CMD "mplayer"
#define MPLAYER_INPUT_FILE "/tmp/mplayer_cmd"
#define MPLAYER_PLAYLIST "playlist.m3u"

// 定义控制函数类型
typedef void (*control_func)(const char *);

// GTK+控件
GtkWidget *window;
GtkWidget *play_button;
GtkWidget *stop_button;
GtkWidget *pause_button;
GtkWidget *volume_scale;

// mplayer进程ID
pid_t mplayer_pid;

// 创建并启动mplayer的函数
void create_and_start_mplayer() {
    int fd;
    char cmd[256];

    // 创建命令管道
    if ((fd = open(MPLAYER_INPUT_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) {
        perror("open");
        exit(1);
    }

    // 启动mplayer进程
    sprintf(cmd, "%s -quiet -slave -input file=%s -playlist %s &", MPLAYER_CMD, MPLAYER_INPUT_FILE, MPLAYER_PLAYLIST);
    if (!fork()) {
        execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
        exit(0);
    }

    mplayer_pid = getpid();
    sleep(1); // 等待mplayer启动
}

// 向mplayer发送命令的函数
void send_mplayer_command(const char *cmd) {
    int fd;
    char buffer[256];

    sprintf(buffer, "%s\n", cmd);
    if ((fd = open(MPLAYER_INPUT_FILE, O_WRONLY)) < 0) {
        perror("open");
        exit(1);
    }
    write(fd, buffer, strlen(buffer));
    close(fd);
}

// 控制按钮的回调函数
void on_play_clicked(GtkButton *button, gpointer data) {
    send_mplayer_command("loadfile " MPLAYER_PLAYLIST);
    send_mplayer_command("play");
}

void on_stop_clicked(GtkButton *button, gpointer data) {
    send_mplayer_command("stop");
    kill(mplayer_pid, SIGTERM);
}

void on_pause_clicked(GtkButton *button, gpointer data) {
    send_mplayer_command("pause");
}

void on_volume_value_changed(GtkAdjustment *adjustment, gpointer data) {
    gdouble volume = adjustment->value;
    char cmd[256];
    sprintf(cmd, "volume %d", (int)(volume * 100));
    send_mplayer_command(cmd);
}

// 主函数
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Music Player");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    play_button = gtk_button_new_with_label("Play");
    stop_button = gtk_button_new_with_label("Stop");
    pause_button = gtk_button_new_with_label("Pause");
    volume_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
    gtk_scale_set_value_pos(GTK_SCALE(volume_scale), GTK_POS_TOP);
    gtk_scale_set_value(GTK_SCALE(volume_scale), 50);

    g_signal_connect(play_button, "clicked", G_CALLBACK(on_play_clicked), NULL);
    g_signal_connect(stop_button, "clicked", G_CALLBACK(on_stop_clicked), NULL);
    g_signal_connect(pause_button, "clicked", G_CALLBACK(on_pause_clicked), NULL);
    g_signal_connect(volume_scale, "value-changed", G_CALLBACK(on_volume_value_changed), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_box_pack_start(GTK_BOX(vbox), play_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), stop_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), pause_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), volume_scale, FALSE, FALSE, 0);

    gtk_widget_show_all(window);

    create_and_start_mplayer();

    gtk_main();

    return 0;
}