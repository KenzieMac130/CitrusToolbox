import time
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
from watchdog.events import FileMovedEvent
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
		if event.is_directory:
			return
		if '~' in event.src_path:
			return
		if '.lock' in event.src_path:
			return
		if type(event) == FileMovedEvent:
			if '~' in event.dest_path:
				return
		print(type(event), event.src_path)
		subprocess.run(["python", "waf", "build", "-v"])

if __name__ == '__main__':
	print("Awaiting changes...")
	w = Watcher()
	w.run()