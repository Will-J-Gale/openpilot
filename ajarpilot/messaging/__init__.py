import os
import capnp
from pathlib import Path
capnp.remove_import_hook()

messaging_dir = Path(__file__).parent
log = capnp.load(os.path.join(str(messaging_dir), "log.capnp"))