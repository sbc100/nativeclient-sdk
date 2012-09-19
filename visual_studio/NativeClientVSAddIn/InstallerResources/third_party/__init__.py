"""Third party includes for the installer.

Module contains:
etree -- The ElementTree XML library.  This is a subset of the full
       ElementTree XML release.

"""


__all__ = ["etree"]

_MINIMUM_XMLPLUS_VERSION = (0, 8, 4)


try:
    import _xmlplus
except ImportError:
    pass
else:
    try:
        v = _xmlplus.version_info
    except AttributeError:
        # _xmlplus is too old; ignore it
        pass
    else:
        if v >= _MINIMUM_XMLPLUS_VERSION:
            import sys
            _xmlplus.__path__.extend(__path__)
            sys.modules[__name__] = _xmlplus
        else:
            del v
