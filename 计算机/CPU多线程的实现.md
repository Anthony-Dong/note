# CPU 多线程的实现

## 1. 时间片

[时间片-百度百科](https://baike.baidu.com/item/%E6%97%B6%E9%97%B4%E7%89%87/6525414?fr=aladdin)

**时间片**（timeslice）又称为“量子（quantum）”或“处理器片（processor slice）”是分时操作系统分配给每个正在运行的[进程](https://baike.baidu.com/item/进程)微观上的一段CPU时间（在抢占[内核](https://baike.baidu.com/item/内核)中是：从进程开始运行直到被抢占的时间）。现代操作系统（如：[Windows](https://baike.baidu.com/item/Windows)、[Linux](https://baike.baidu.com/item/Linux)、[Mac OS X](https://baike.baidu.com/item/Mac OS X)等）允许同时运行多个进程 —— 例如，你可以在打开音乐播放器听音乐的同时用[浏览器](https://baike.baidu.com/item/浏览器)浏览[网页](https://baike.baidu.com/item/网页)并下载文件。事实上，虽然一台[计算机](https://baike.baidu.com/item/计算机)通常可能有多个[CPU](https://baike.baidu.com/item/CPU)，但是同一个CPU永远不可能真正地同时运行多个任务。在只考虑一个CPU的情况下，这些进程“看起来像”同时运行的，实则是轮番穿插地运行，由于时间片通常很短（在[Linux](https://baike.baidu.com/item/Linux)上为5ms－800ms,Windows是上20ms），用户不会感觉到。



## 2. 上下文切换

[上下文切换-百度百科](https://baike.baidu.com/item/%E4%B8%8A%E4%B8%8B%E6%96%87%E5%88%87%E6%8D%A2/4842616?fr=aladdin)

上下文切换，有时也称做进程切换或任务切换，是指[CPU](https://baike.baidu.com/item/CPU) 从一个[进程](https://baike.baidu.com/item/进程/382503)或线程切换到另一个进程或线程。

在上下文切换过程中，CPU会停止处理当前运行的程序，并保存当前程序运行的具体位置以便之后继续运行。从这个角度来看，上下文切换有点像我们同时阅读几本书，在来回切换书本的同时我们需要记住每本书当前读到的页码。在程序中，上下文切换过程中的“页码”信息是保存在进程控制块（PCB）中的。PCB还经常被称作“切换帧”（switchframe）。“页码”信息会一直保存到CPU的内存中，直到他们被再次使用。

在三种情况下可能会发生上下文切换：中断处理，多任务处理，用户态切换。在中断处理中，其他程序”打断”了当前正在运行的程序。当CPU接收到中断请求时，会在正在运行的程序和发起中断请求的程序之间进行一次上下文切换。在多任务处理中，CPU会在不同程序之间来回切换，每个程序都有相应的处理时间片，CPU在两个时间片的间隔中进行上下文切换。对于一些操作系统，当进行用户态切换时也会进行一次上下文切换，虽然这不是必须的。

操作系统或者计算机硬件都支持上下文切换。一些现代操作系统通过系统本身来控制上下文切换，整个切换过程中并不依赖于硬件的支持，这样做可以让操作系统保存更多的上下文切换信息





其实概念就这几个, 你要是分配时间片, 那么就需要一个东西去保存上下文状态(PCB) , 所以有利也有弊 , 大量的上下文切换, 就需要保存大量的上下文 , 保存和切换都需要消耗大量的时间, 所以各有取舍. 

