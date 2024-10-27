import logging
import threading
import time
from queue import Queue
from threading import Event


class OnThreadExecutor:
    __instance_lock = threading.Lock()
    __instances = {}
    __keys = []

    def __init__(self, key):
        if not key in OnThreadExecutor.__keys:
            raise Exception('Can not find instance')
        if key in OnThreadExecutor.__instances:
            if not self == OnThreadExecutor.__instances[key]:
                raise Exception('Invalid instance')
        self.key = key
        self.__canceled = False
        self.__execLock = threading.Lock()
        self.__queueLock = threading.Lock()
        self.__actions = Queue()
        self.__are = Event()
        self.__task = threading.Thread(target=self.__func_task).start()

    @staticmethod
    def get_instance(key):
        with OnThreadExecutor.__instance_lock:
            if key in OnThreadExecutor.__instances:
                return OnThreadExecutor.__instances[key]
            OnThreadExecutor.__keys.append(key)
            OnThreadExecutor.__instances.update({key: OnThreadExecutor(key)})
            return OnThreadExecutor.__instances[key]

    def __func_task(self):
        while not self.__canceled:
            self.__are.wait()
            with self.__execLock:
                try:
                    is_execute = False
                    with self.__queueLock:
                        if not self.__actions.empty():
                            act = self.__actions.get()
                            is_execute = True
                        self.__are.clear()
                    if is_execute:
                        act()
                    with self.__queueLock:
                        if not self.__actions.empty():
                            self.__are.set()
                except Exception as e:
                    es = 'OnThreadExecutor %s : %s' % (self.key, str(e))
                    logging.error(es)
                    print(es)
                    self.__are.set()

    def cancel(self):
        self.__canceled = True
        with self.__queueLock:
            while not self.__actions.empty():
                self.__actions.get()
        self.__are.set()

    def submit(self, act):
        with self.__queueLock:
            if self.__canceled:
                raise Exception('Task canceled')
            self.__actions.put(act)
            self.__are.set()
            return self

    def wait(self):
        task_completed = False
        while not task_completed:
            with self.__queueLock:
                task_completed = self.__canceled or self.__actions.empty()
            time.sleep(0.001)
        with self.__execLock:
            pass

    # def dispose(self):
    #     self.cancel()
    #     self.__are.set()
    #     self.__task.join()
