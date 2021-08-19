import time
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
import os
import subprocess

class Watcher:

    def __init__(self):
        self.observer = Observer()

    def run(self):
        event_handler = Handler()
        self.observer.schedule(event_handler, os.getcwd(), recursive=True)
        self.observer.start()
        try:
            while True:
                time.sleep(5)
        except:
            self.observer.stop()

        self.observer.join()


class Handler(FileSystemEventHandler):

    @staticmethod
    def on_any_event(event):
        subprocess.run(["python", "waf", "build", "-v"])

if __name__ == '__main__':
    print("Awaiting changes...")
    w = Watcher()
    w.run()