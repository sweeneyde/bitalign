from setuptools import Extension, setup

setup(
    ext_modules=[
        Extension(
            name="bitalign",
            sources=["bitalign.c", "bitalignmodule.c"],
            extra_compile_args=["-O2"],
        ),
    ]
)
