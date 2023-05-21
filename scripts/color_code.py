from enum import Enum

class ColorCode(Enum):
    """ANSI color codes for terminal output"""
    BLACK = "\033[0;{}0m"
    RED = "\033[0;{}1m"
    GREEN = "\033[0;{}2m"
    BROWN = "\033[0;{}3m"
    BLUE = "\033[0;{}4m"
    PURPLE = "\033[0;{}5m"
    CYAN = "\033[0;{}6m"
    LIGHT_GRAY = "\033[0;{}7m"
    DARK_GRAY = "\033[1;{}0m"
    LIGHT_RED = "\033[1;{}1m"
    LIGHT_GREEN = "\033[1;{}2m"
    YELLOW = "\033[1;{}3m"
    LIGHT_BLUE = "\033[1;{}4m"
    LIGHT_PURPLE = "\033[1;{}5m"
    LIGHT_CYAN = "\033[1;{}6m"
    LIGHT_WHITE = "\033[1;{}7m"
    END = "\033[0m"

    def foreground(self) -> str:
        return self.value.format('3')

    def background(self) -> str:
        return self.value.format('4')