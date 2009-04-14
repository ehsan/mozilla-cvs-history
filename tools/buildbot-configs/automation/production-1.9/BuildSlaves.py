from buildbot.buildslave import BuildSlave

SlaveList = [
 BuildSlave("production-1.9-master","",max_builds=1),
 BuildSlave("fx-linux-1.9-slave2","",max_builds=1),
 BuildSlave("fx-win32-1.9-slave2", "",max_builds=1),
 BuildSlave("fx-linux64-1.9-slave1", "",max_builds=1),
 BuildSlave("fx-linux-1.9-slave4", "",max_builds=1),
 BuildSlave("fx-win32-1.9-slave4", "",max_builds=1),
 BuildSlave("fx-mac-1.9-slave2", "",max_builds=1),
]

