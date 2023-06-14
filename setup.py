from setuptools import Extension, setup

setup(
    ext_modules=[
        Extension(
            name="_bitalign",
            sources=["src/_bitalign.c", "src/_bitalignmodule.c"],
        ),
    ],
    packages=["bitalign"]
)
