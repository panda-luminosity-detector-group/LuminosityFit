import attr


@attr.s
class AlignmentParameters:
    misalignment_matrices_path: str = attr.ib(default="")
    use_point_transform_misalignment: bool = attr.ib(default=False)
    alignment_matrices_path: str = attr.ib(default="")
