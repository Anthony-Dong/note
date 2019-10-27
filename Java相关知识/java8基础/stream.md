# stream

## parallel

> ​	并行流 , 他是 一种 forkjoin\

```java
		 List<Integer> lists = new ArrayList<>();
        lists.add(1);
        lists.add(2);
        lists.add(3);
        lists.add(4);
        lists.add(5);
        lists.add(6);

		// 只会调用一次初始值
        Integer reduce = lists.stream().reduce(1, (a, b) -> a + b);
		
        System.out.println(reduce);
		
		//由于是并行流,所以他不是线程安全的,同时也不是有序的,他每次都会调用初始值
        Integer reduce2 = lists.parallelStream().reduce(1, (a, b) -> a + b);
		
        System.out.println(reduce2);

		无序的 所以需要,forEachOrdered()使其 有序
		lists.parallelStream().forEachOrdered(System.out::println);

        lists.parallelStream().forEach(System.out::println);
```

