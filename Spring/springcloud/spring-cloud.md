# SpringCloud day01

## 1. Spring 事件/监听器器模式

ApplicationEvent  /  ApplicationListener

### 1. 第一种方式(API)

```java
package com.springcloud.event;

import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.context.annotation.AnnotationConfigApplicationContext;

/**
 * ClassName:SpringEventListenerDemo
 * Package:com.springcloud.event
 * @date:2019/8/19 14:48
 * Description: Spring 事件/监听器器模式
 * @author: <a href='mailto:fanhaodong516@qq.com'>Anthony</a>
 */

public class SpringEventListenerDemo {


    public static void main(String[] args) {
        /**
         * 创建一个上下文对象 其 父类都是 ConfigApplicationContext ;这里使用的注解上下文
         */
        AnnotationConfigApplicationContext context = new AnnotationConfigApplicationContext();

        //2. 添加 监听者
        context.addApplicationListener(new MyApplicationListener());

        //1.开启应用的上下文
        context.refresh();

        //3. 发布事件
        context.publishEvent(new MyApplicationEvent("hello world"));
        context.publishEvent(new MyApplicationEvent("hello world1"));
        context.publishEvent(new MyApplicationEvent("hello world2"));

    }

    /**
     * 创建一个事件监听器
     */
    private static class MyApplicationListener implements  ApplicationListener<MyApplicationEvent>{

        @Override
        public void onApplicationEvent(MyApplicationEvent event) {
            System.out.println("event.getSource() = " + event.getSource());
        }
    }

    /**
     * 创建 一个事件对象
     */
    private static class MyApplicationEvent extends ApplicationEvent {

        /**
         * Create a new ApplicationEvent.
         *
         * @param source the object on which the event initially occurred (never {@code null})
         */
        public MyApplicationEvent(Object source) {
            super(source);
        }
    }
}

```

### 2.第二种方式(注解方式)

```java
package com.springcloud.event;

import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.context.annotation.AnnotationConfigApplicationContext;
import org.springframework.stereotype.Component;

/**
 * ClassName:SpringEventListenerDemo
 * Package:com.springcloud.event
 * @date:2019/8/19 14:48
 * Description: Spring 事件/监听器器模式
 * @author: <a href='mailto:fanhaodong516@qq.com'>Anthony</a>
 */

public class SpringEventListenerDemo {


    public static void main(String[] args) {
        /**
         * 创建一个上下文对象 其 父类都是 ConfigApplicationContext ;这里使用的注解上下文
         */
        AnnotationConfigApplicationContext context = new AnnotationConfigApplicationContext();

        //2. 添加 监听者
//        context.addApplicationListener(new MyApplicationListener());
        // 注册 注解bean对象
        context.register(MyApplicationListener.class);
        //1.开启上下文
        context.refresh();

        //3. 发布事件
        context.publishEvent(new MyApplicationEvent("hello world"));
        context.publishEvent(new MyApplicationEvent("hello world1"));
        context.publishEvent(new MyApplicationEvent("hello world2"));

    }

    /**
     * 创建一个事件监听器
     */
    @Component
    private static class MyApplicationListener implements  ApplicationListener<MyApplicationEvent>{

        @Override
        public void onApplicationEvent(MyApplicationEvent event) {
            System.out.println("MyApplicationListener receive event source = " + event.getSource());
        }
    }

    /**
     * 创建 一个事件对象
     */
    private static class MyApplicationEvent extends ApplicationEvent {

        /**
         * Create a new ApplicationEvent.
         *
         * @param source the object on which the event initially occurred (never {@code null})
         */
        public MyApplicationEvent(Object source) {
            super(source);
        }
    }
}

```

### 3. 第三种方式(构造器)

```java
package com.springcloud.event;

import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.context.annotation.AnnotationConfigApplicationContext;
import org.springframework.stereotype.Component;

/**
 * ClassName:SpringEventListenerDemo
 * Package:com.springcloud.event
 * @date:2019/8/19 14:48
 * Description: Spring 事件/监听器器模式
 * @author: <a href='mailto:fanhaodong516@qq.com'>Anthony</a>
 */

public class SpringEventListenerDemo {


    public static void main(String[] args) {
        /**
         * 创建一个上下文对象 其 父类都是 ConfigApplicationContext ;这里使用的注解上下文
         * 
         * 创建这个对象的时候 会 自动注入  和 开启上下文  用构造器
         */
        AnnotationConfigApplicationContext context = new AnnotationConfigApplicationContext(MyApplicationListener.class);

        //2. 添加 监听者
//        context.addApplicationListener(new MyApplicationListener());
        // 注册 注解bean对象
//        context.register(MyApplicationListener.class);
        //1.开启上下文
//        context.refresh();

        //3. 发布事件
        context.publishEvent(new MyApplicationEvent("hello world"));
        context.publishEvent(new MyApplicationEvent("hello world1"));
        context.publishEvent(new MyApplicationEvent("hello world2"));

    }

    /**
     * 创建一个事件监听器
     */
    @Component
    private static class MyApplicationListener implements  ApplicationListener<MyApplicationEvent>{

        @Override
        public void onApplicationEvent(MyApplicationEvent event) {
            System.out.println("MyApplicationListener receive event source = " + event.getSource());
        }
    }

    /**
     * 创建 一个事件对象
     */
    private static class MyApplicationEvent extends ApplicationEvent {

        /**
         * Create a new ApplicationEvent.
         *
         * @param source the object on which the event initially occurred (never {@code null})
         */
        public MyApplicationEvent(Object source) {
            super(source);
        }
    }
}

```

### 4.模式解释

![image](https://tyut.oss-cn-beijing.aliyuncs.com/edu/picture/spring%E4%BA%8B%E4%BB%B6%E7%9B%91%E5%90%AC%E6%A8%A1%E5%BC%8F.png-template01)

