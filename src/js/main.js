function render() {
    print('Hello, VitaDeck!');
    print('Running JavaScript:');

    // Arrays concatenate to strings
    print('[] + []: ' + ([] + []));

    // String concatenation before subtraction
    print('"2" + "2" - "2": ' + ("2" + "2" - "2"));

    // Booleans coerce to numbers
    print('true + true: ' + (true + true));

    // NaN is never equal to itself
    print('NaN === NaN: ' + (NaN === NaN));

    // Floating point precision
    print('0.1 + 0.2 === 0.3: ' + (0.1 + 0.2 === 0.3));

    // Loose vs strict equality
    print('null == undefined: ' + (null == undefined));
    print('null === undefined: ' + (null === undefined));
}