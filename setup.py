from setuptools import Extension, setup

setup(
    name='bitalign',
    version='0.3.0',
    author="Dennis Sweeney",
    author_email="sweeney.427@osu.edu",
    url="https://github.com/sweeneyde/bitalign",
    description="Shift some bits and see how they line up best.",
    ext_modules=[
        Extension(
            name="_bitalign",
            sources=["src/_bitalign.c", "src/_bitalignmodule.c"],
        ),
    ],
    packages=["bitalign"]
)
