from setuptools import setup, find_packages

setup(
    name="alphabet-lang",
    version="1.0.0",
    packages=find_packages(),
    py_modules=['alphabet'],
    install_requires=[],
    entry_points={
        'console_scripts': [
            'alphabet=alphabet:main',
        ],
    },
    author="Fraol Teshome",
    author_email="fraolteshome444@gmail.com",
    description="The Alphabet Programming Language",
    python_requires='>=3.10',
)
