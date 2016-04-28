BannerGUI1.0(win32):
基本实现了简单的GUI，图片、标题、说明文字均可以用文件选择对话框选择，当然这些都要求是图片格式
可以选择横版或者竖版
然后单击开始按钮即可

BannerGUI2.0:
用户可以选择默认版式，或者自定义版式
默认版式附加了图片参考

Banner3.0:
改善了界面布局排版
在release版本下运行时，会不时跳出命令行窗口，原因是load_bitmap()和save_bitmap()里面调用system()执行cmd命令
后改用CreateProcess()代替