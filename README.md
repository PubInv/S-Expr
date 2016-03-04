# S-Expr
An Arduino library implementing a simple and incomplete S-Expr functionality, most useful for serial communication

## Status

At the time of this writing: buggy, incomplete, weird, and poorly written.

## Motivation

Often in using an Arduino, particularly for robotics, one wishes to communicate commands to the Arudino. Ordinarily this is done with serial communications. In the simplest case you simply send a character to teh Arduino, and the Arduino inteprets it as a command.

But when the set of comands and complexity of those commands grows greater, once must implement a tiny protocol or language of some kind. For example, I am motivated by the need to control a 6-channel actuator controller, which must report the position of the actuators, and allow them to be moved to arbitrary positions.

I personally believe it is a mistake to create a new protocol when we have at hand such protocols aleady. JSON is perhaps the most popular at present.

However, JSON is merely a syntactic sugar on top of S-Expressions (from LISP.) Since I am programming my system in LISP already, I have chosen to do this in the LISP-style, although I suspect it would be more reusable and understandable if I did int in JSON.

Basically you want to send on the serial port commands like:

(status)
(m 400 300 500 600 600 1000)
(m (0 100) (5 100))

That is, whether we like it or not, we have to have SOME format for transmiting the commands, and it might as well be S-Expressions.

## Limitations

Comparted to LISP or Scheme, the S-Expressions here are missing:

* floats,
* dotted expressions,
* symbls,
* and many more.

Nonetheless it is useful for some limited purposes.

## Dangers

At the time of this writing, there are probably memory leaks that would absolutely preclude using this in any production system.  However, I will soon fix these---unless the gentle reader beats me to it.

## Use

Basically, put this directory (S-Expr) in the Arduino/libraries directory. Then in your Arduino code put:

```C
#include <S-Expr.h>
```

To use, do something like:

```C
sexpr* s = parse(str);

if (String("m").equals(nth(s,0))) {



}

```

## Tests

I suspect and Arduino expect can explain a better way of doing this.

Because it is tedious using Arduino-Mk to put a .ino file that allows testing in the library code,
I have created a separate repo, S-Expr-Test, which contains a .ino file which you can run directly to execute the automated tests that I used to develop this.
