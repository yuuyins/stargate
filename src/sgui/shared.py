import datetime
import os

from sglib import constants
from sglib.lib import util
from sglib.log import LOG
from sgui.sgqt import *


AUDIO_ITEM_SCENE_HEIGHT = 900.0
AUDIO_ITEM_SCENE_WIDTH = 3600.0
AUDIO_ITEM_SCENE_RECT = QtCore.QRectF(
    0.0,
    0.0,
    AUDIO_ITEM_SCENE_WIDTH,
    AUDIO_ITEM_SCENE_HEIGHT,
)

# These are dynamically assigned by stargate.py so that
# hosts can access them from this module
MAIN_WINDOW = None
HOST_MODULES = None
APP = None
TRANSPORT = None
IS_PLAYING = False
IS_RECORDING = False
PLUGIN_UI_DICT = None
CURRENT_HOST = 0
TOOLTIPS_ENABLED = util.get_file_setting("tooltips", int, 1)
MEMORY_ENTROPY = datetime.timedelta(minutes=0)
MEMORY_ENTROPY_LIMIT = datetime.timedelta(minutes=30)
MEMORY_ENTROPY_UIDS = set()
CC_CLIPBOARD = None


def on_ready():
    LOG.info("Engine sent 'ready' message")
    for mod in HOST_MODULES:
        mod.on_ready()
    constants.READY = True

def clean_audio_pool():
    result = set()
    for host in HOST_MODULES:
        uids = host.active_audio_pool_uids()
        LOG.info(f"{host}, {uids}")
        result.update(uids)

    if util.USE_HUGEPAGES:
        for f_uid in (x for x in result if x in MEMORY_ENTROPY_UIDS):
            MEMORY_ENTROPY_UIDS.remove(f_uid)
    #invert
    audio_pool = constants.PROJECT.get_audio_pool()
    LOG.info(f"{audio_pool}")
    f_result = [
        x.uid
        for x in audio_pool.pool
        if x.uid not in result
    ]
    LOG.info(f"clean_audio_pool: freeing {f_result}'")
    if f_result:
        f_msg = "|".join(str(x) for x in sorted(f_result))
        constants.IPC.clean_audio_pool(f_msg)

    if util.USE_HUGEPAGES:
        for f_uid in (x for x in f_result if x not in MEMORY_ENTROPY_UIDS):
            MEMORY_ENTROPY_UIDS.add(f_uid)
            f_sg = constants.PROJECT.get_sample_graph_by_uid(f_uid)
            f_delta = datetime.timedelta(seconds=f_sg.length_in_seconds)
            if add_entropy(f_delta):
                restart_engine()
                break

def add_entropy(a_timedelta):
    """ Use this to restart the engine and clean up the wav pool memory

        This returns a bool, to avoid restarting the engine at an
        inopportune time.  It is the responsibility of the caller to
        also call
    """
    global MEMORY_ENTROPY
    MEMORY_ENTROPY += a_timedelta
    if MEMORY_ENTROPY > MEMORY_ENTROPY_LIMIT:
        LOG.info("Recording entropy exceeded, restarting engine "
            "to clean and defragment memory")
        MEMORY_ENTROPY = datetime.timedelta(minutes=0)
        return True
    else:
        return False

def restart_engine():
    close_engine()
    reopen_engine()

def prepare_to_quit():
    global MAIN_WINDOW, TRANSPORT
    MAIN_WINDOW = TRANSPORT = None

def set_window_title():
    if not MAIN_WINDOW:
        return
    dirname = constants.PROJECT.project_folder
    if util.IS_WINDOWS:
        dirname = dirname.replace('/', '\\')
    MAIN_WINDOW.setWindowTitle(
        'Stargate - {}'.format(
            os.path.join(
                dirname,
                f'{util.MAJOR_VERSION}.project',
            )
        ),
    )

