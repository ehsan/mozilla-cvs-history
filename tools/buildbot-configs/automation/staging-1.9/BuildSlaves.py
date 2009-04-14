from buildbot.buildslave import BuildSlave

SlaveList = [
 BuildSlave("staging-1.9-master","",max_builds=1),
 BuildSlave("fx-linux-1.9-slave1","",max_builds=1),
 BuildSlave("fx-linux-1.9-slave3","",max_builds=1),
 BuildSlave("fx-linux-1.9-slave4","",max_builds=1),
 BuildSlave("fx-win32-1.9-slave1", "",max_builds=1),
 BuildSlave("fx-win32-1.9-slave3", "",max_builds=1),
 BuildSlave("fx-win32-1.9-slave4", "",max_builds=1),
 BuildSlave("mini-test", "",max_builds=1),
 BuildSlave("fx-mac-1.9-slave1", "",max_builds=1),
]
