from pathlib import Path
from typing import Optional

import attr
from attr import field


@attr.s
class AlignmentParameters:
    alignment_matrices_path: Optional[Path] = field(default=None)
    misalignment_matrices_path: Optional[Path] = field(default=None)
    use_point_transform_misalignment: bool = attr.ib(default=False)
