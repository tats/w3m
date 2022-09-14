import sys
import pathlib
import re
from typing import NamedTuple, Union
from enum import Enum, auto

MACRO_PATTERN = re.compile(r'^#\s*(\S+)\s*(\S.*)?')


class Include(NamedTuple):
    value: str


class Define(NamedTuple):
    value: str


class If(NamedTuple):
    value: str


class Ifdef(NamedTuple):
    value: str


class Ifndef(NamedTuple):
    value: str


class Elif(NamedTuple):
    value: str


class Else(NamedTuple):
    pass


class Endif(NamedTuple):
    pass


def parse_macro(l: str) -> Union[None, Include, Define, If, Ifdef, Ifndef, Else, Elif, Endif]:
    m = MACRO_PATTERN.match(l)
    if m:
        match m.group(1):
            case 'include':
                return Include(m.group(2))
            case 'define':
                return Define(m.group(2))
            case 'if':
                return If(m.group(2))
            case 'ifndef':
                return Ifndef(m.group(2))
            case 'ifdef':
                return Ifdef(m.group(2))
            case 'elif':
                return Elif(m.group(2))
            case 'else':
                return Else()
            case 'endif':
                return Endif()


def main(path: pathlib.Path):
    for l in path.read_text().splitlines():
        if l.startswith('#'):
            match parse_macro(l):
                case Include() as m:
                    pass
                    # print(f'[INCLUDE] => {m}')
                case Define() as m:
                    pass
                    # print(f'[DEFINE] => {m}')
                case If() as m:
                    pass
                    # print(f'[If] => {m}')
                case Ifndef() as m:
                    pass
                case Ifdef() as m:
                    pass
                case Elif() as m:
                    pass
                case Else() as m:
                    pass
                case Endif() as m:
                    pass
                case _:
                    print(l)
                    # print(l)


if __name__ == '__main__':
    main(pathlib.Path(sys.argv[1]))
