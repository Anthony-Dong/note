# xxl-job

## 新版本

核心逻辑没变：

```java
@RequestMapping("/remove")
@ResponseBody
public ReturnT<String> remove(int id) {
  return xxlJobService.remove(id);
}

@RequestMapping("/stop")
@ResponseBody
public ReturnT<String> pause(int id) {
  return xxlJobService.stop(id);
}

@RequestMapping("/start")
@ResponseBody
public ReturnT<String> start(int id) {
  return xxlJobService.start(id);
}

@RequestMapping("/trigger")
@ResponseBody
//@PermessionLimit(limit = false)
public ReturnT<String> triggerJob(int id, String executorParam) {
  // force cover job param
  if (executorParam == null) {
    executorParam = "";
  }

  JobTriggerPoolHelper.trigger(id, TriggerTypeEnum.MANUAL, -1, null, executorParam);
  return ReturnT.SUCCESS;
}
```



核心逻辑， start  ， stop ， trigger



```java
    /**
     * add trigger
     */
    public void addTrigger(final int jobId, final TriggerTypeEnum triggerType, final int failRetryCount, final String executorShardingParam, final String executorParam) {

        // choose thread pool
        ThreadPoolExecutor triggerPool_ = fastTriggerPool;
        AtomicInteger jobTimeoutCount = jobTimeoutCountMap.get(jobId);
        if (jobTimeoutCount!=null && jobTimeoutCount.get() > 10) {      // job-timeout 10 times in 1 min
            triggerPool_ = slowTriggerPool;
        }

        // trigger
        triggerPool_.execute(new Runnable() {
            @Override
            public void run() {

                long start = System.currentTimeMillis();

                try {
                    // do trigger , 还是在这里
                    XxlJobTrigger.trigger(jobId, triggerType, failRetryCount, executorShardingParam, executorParam);
                } catch (Exception e) {
                    logger.error(e.getMessage(), e);
                } finally {

                    // check timeout-count-map
                    long minTim_now = System.currentTimeMillis()/60000;
                    if (minTim != minTim_now) {
                        minTim = minTim_now;
                        jobTimeoutCountMap.clear();
                    }

                    // incr timeout-count-map
                    long cost = System.currentTimeMillis()-start;
                    if (cost > 500) {       // ob-timeout threshold 500ms
                        AtomicInteger timeoutCount = jobTimeoutCountMap.put(jobId, new AtomicInteger(1));
                        if (timeoutCount != null) {
                            timeoutCount.incrementAndGet();
                        }
                    }

                }

            }
        });
    }
```











## 旧版本

