from buildbot.status.builder import FAILURE, SUCCESS
from buildbot.steps.shell import ShellCommand

class UpdateVerify(ShellCommand):
    def evaluateCommand(self, cmd):
        for line in cmd.logs['stdio'].getText().split("\n"):
            if line.startswith('FAIL'):
                return FAILURE

        return SUCCESS
