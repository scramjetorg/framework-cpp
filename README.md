Scramjet Framework C++
==================

<p align="center">
    <a><img src="https://img.shields.io/github/license/scramjetorg/framework-cpp?color=green&style=plastic" alt="GitHub license" /></a>
    <a><img src="https://img.shields.io/github/v/tag/scramjetorg/framework-cpp?label=version&color=blue&style=plastic" alt="version" /></a>
    <a><img src="https://img.shields.io/github/stars/scramjetorg/framework-cpp?color=pink&style=plastic" alt="GitHub stars" /></a>
    <a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=7F7V65C43EBMW">
        <img src="https://img.shields.io/badge/Donate-PayPal-green.svg?color=yellow&style=plastic" alt="Donate" />
    </a>
</p>
<p align="center">⭐ Star us on GitHub — it motivates us a lot! 🚀 </p>
<p align="center">
    <img src="https://assets.scramjet.org/images/framework-logo-256.svg" width="420" alt="Scramjet Framework">
</p>

Scramjet is a simple reactive stream programming framework. The code is written by chaining functions that transform the streamed data, including well known map, filter and each.

The main advantage of Scramjet is running asynchronous operations on your data streams concurrently. It allows you to perform the transformations both synchronously and asynchronously by using the same API - so now you can "map" your stream from whatever source and call any number of API's consecutively.

[Originally written](https://github.com/scramjetorg/scramjet) on top of node.js
object streams, Scramjet is now being ported into C++. This is what is
happening in this repository. There is also [JavaScript/TypeScript](https://github.com/scramjetorg/framework-js) and [Python](https://github.com/scramjetorg/framework-python) version available.

**We are open to your feedback!** We encourage you to report issues with any ideas, suggestions and features you would like to see in this version. You can also upvote (`+1`) existing ones to show us the direction we should take in developing Scramjet Framework.

## Table of contents

- [Scramjet Framework C++](#scramjet-framework-c)
  - [Table of contents](#table-of-contents)
  - [Installation](#installation)
  - [Usage](#usage)
  - [Requesting Features](#requesting-features)
  - [Reporting bugs](#reporting-bugs)
  - [Contributing](#contributing)
  - [Development Setup](#development-setup)

## Installation

To use Ifca interface library just clone framework-cpp repository and add include directory to your project then simply include headers of your need.

## Usage

Write and read chunks:
      
```C++
std::shared_future<void> write(Chunk chunk)
```
- writes chunk of type Chunk to ifca algorithm (must be the same type as input type of first transform). Returns drain state- if we've surpressed [maxParallel] chunk processed, it will return shared_future to wait for draining stream, otherwise returns already set shared_future.

```C++
std::future<OutputType> read()
```
- Returns future of processed chunk of type OutputType- type returned by last transform in chain. Future is set after transformations are finished.

Transform chunks:

To add Transform to processing chain use:
```C++
 addTransform(Transform)
```
Transforms accept any callable object having operator(ChunkType value) overloaded- functions, member functions, lambdas, std::function, functional objects.

Possible values are:
```C++
 each(Callable function)
```
- Calls Callable(chunk) and passes result of callable further. 
```C++
 map(Callable function)
```
- Calls Callable(chunk) and passes same chunk further without taking result of callable.
```C++
 filter(Predicate function)
```
- Calls Predicate(chunk)- must return bool value. If predicate is true passes chunk further, otherwise filters out chunk from flow.

Example usage:
```C++
  // for readability only
  using namespace ifca; 

  auto incrementByOne = [](int chunk) {
    return chunk + 1;
  };
  auto showChunk = [](int chunk) {
    std::cout << chunk << "\n";
    return chunk;
  };
  auto filterOddChunk = [](int chunk){
      return (chunk % 2 == 0);
  }

  const unsigned int maxParallel = 8;
  
  auto ifca = Ifca().addTransform(map(incrementByOne)).addTransform(each(showChunk)).addTransform(filter(filterOddChunk));

  auto inputReader = std::async(std::launch::async, [&]() {
    int c;
    while (std::cin >> c) {
      ifca.write(c).wait();
    }
    ifca.end();
  });

  auto outputReader = std::async(std::launch::async, [&]() {
      while (true) 
        std::cout << ifca.read().get();
  });
  outputReader.wait();

```

## Requesting Features

Anything missing? Or maybe there is something which would make using Scramjet Framework much easier or efficient? Don't hesitate to fill up a [new feature request](https://github.com/scramjetorg/framework-cpp/issues/new)! We really appreciate all feedback.

## Reporting bugs

If you have found a bug, inconsistent or confusing behavior please fill up a [new bug report](https://github.com/scramjetorg/framework-cpp/issues/new).

## Contributing

You can contribute to this project by giving us feedback ([reporting bugs](#reporting-bugs) and [requesting features](#reporting-features)) and also by writing code yourself!

The easiest way is to [create a fork](https://docs.github.com/en/get-started/quickstart/fork-a-repo) of this repository and then [create a pull request](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/proposing-changes-to-your-work-with-pull-requests/creating-a-pull-request-from-a-fork) with all your changes. In most cases, you should branch from and target `main` branch.

Please refer to [Development Setup](#development-setup) section on how to setup this project.

## Development Setup

1\. Clone this repo:

```bash
git@github.com:scramjetorg/framework-cpp.git
```

2\. List presets available on your OS:

```bash
cmake --list-presets=all .
```

It will result with a similar list:

```bash
Available configure presets:

  "linux-debug"         - Linux Debug
  "linux-test-coverage" - Linux Test Coverage
  "linux-release"       - Linux Release
```

3\. Build `make` according to specific preset:

```bash
cmake --preset linux-debug
```

4\. Build project using `make` file from previous step:

```bash
cd out/build/linux-debug/
cmake --build .
```

5\. Run tests (only debug presets)

```bash
./tests/IFCA_tests
```

5\. Run code coverage raport:

```bash
cmake --preset linux-test-coverage
cd out/build/linux-test-coverage
make IFCA_coverage
```

Raport will be under `out/build/linux-test-coverage/IFCA-coverage` folder, open `index.html` file to check specific results.
