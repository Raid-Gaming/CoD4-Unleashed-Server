assertThat(statement, msg) {
  if (!statement) {
    print("^1Fail:^1 " + msg + "\n");
  }
}

describe(name) {
  if (!isDefined(level.testSuite)) {
    level.testSuite = [];
  }

  spec = [];
  spec["name"] = name;
  spec["tests"] = [];

  index = level.testSuite.size;
  level.testSuite[index] = spec;

  return index;
}

it(spec, name, callback) {
  self test(spec, name, callback);
}

run() {
  assertEx(isDefined(level.testSuite), "no test suite has been set up");

  if (!isDefined(level.testSuite)) {
    print("^1Test suite has not been set up correctly\n");
    return;
  }

  print("^2Running unit tests\n");
  for (i = 0; i < level.testSuite.size; i++) {
    spec = level.testSuite[i];
    print("^2" + spec["name"] + "\n");
    for (j = 0; j < spec["tests"].size; j++) {
      testCase = spec["tests"][j];
      print("--> " + testCase["name"] + "\n");
      [[testCase["function"]]]();
    }
  }
}

test(spec, name, callback) {
  assertEx(isDefined(level.testSuite), "no test suite has been set up");

  testCase = [];
  testCase["name"] = name;
  testCase["function"] = callback;

  index = level.testSuite[spec]["tests"].size;
  level.testSuite[spec]["tests"][index] = testCase;
}
