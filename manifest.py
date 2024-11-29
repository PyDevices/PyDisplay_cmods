metadata(
    description="pydisplay_cmods modules",
    version="0.0.0",
)

try:
    include("$(BOARD_DIR)/manifest.py")
except Exception:
    try:
        include("$(PORT_DIR)/boards/manifest.py")
    except Exception:
        try:
            include("$(PORT_DIR)/variants/standard/manifest.py")
        except Exception:
            try:
                include("$(PORT_DIR)/variants/manifest.py")
            except Exception:
                pass
