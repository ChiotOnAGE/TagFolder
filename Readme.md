# Directory Tag Edit

使用本程序可以通过命令行或右键菜单选项给所选文件夹添加或修改 `tags` 属性。通过 `tags` 可以实现的操作包括但不限于：

+ 在 `tags` 中添加备注或者类别，方便查找和管理。
+ 自定义文件夹排列顺序，例如先按照 `tags` 顺序分段显示，同 `tags` 文件夹再按照名称排序。
+ `tags` 也可以用来在浏览时快速标记，方便后续其他程序自动化处理

当前版本可视作 Pinjoy 发布的 BAT, Powershell, VBS 奇美拉 [TagFolder](https://youtu.be/vyFhSdm4gD8) 的 C++ 版本，并在其基础上提升了兼用性和易用性，如：

+ 支持删除 `tags`
+ 支持 UTF-8 编码
+ 支持通过命令行使用
+ 使用时不会弹出命令行窗口
+ 输入框显示当前 `tags` 并选中
+ 多个实例同时工作时无冲突风险

程序仍保留了 VBS 代码，因为使用 [`shell.MoveHere`] 是几个强制立即应用 `desktop.ini` 变化的[奇技淫巧](https://stackoverflow.com/questions/68941080/update-folder-icon-with-desktop-ini-instantly-change-c)中尝试下来唯一奏效的。

预计后续还会增加以下功能：
+ 在弹窗中显示文件夹名方便在多选时进行区分
+ 丰富命令行支持的参数，并生成独立的命令行程序以支持在 headless 情况下使用
+ 输入框支持 `ESC` 和 `Ctrl+A` 等快捷键
