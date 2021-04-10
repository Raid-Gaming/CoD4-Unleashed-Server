// create an array
create() {
  arr = [];
  return arr;
}

// returns true if every element in this array satisfies the provided testing function
every(callback) {
  assertEx(isDefined(self), "caller undefined, should be array");
  assertEx(isArray(self), "caller should be an array");
  assertEx(isDefined(callback), "argument is undefined");

  for (i = 0; i < self.size; i++) {
    if (![[callback]](self[i], i, self)) {
      return false;
    }
  }
  return true;
}

// creates a new array with all of the elements in this array for which the provided filtering functions returns true
filter(callback) {
  assertEx(isDefined(self), "caller undefined, should be array");
  assertEx(isArray(self), "caller should be an array");
  assertEx(isDefined(callback), "argument is undefined");

  arr = [];
  for (i = 0; i < self.size; i++) {
    if ([[callback]](self[i], i, self)) {
      arr[arr.size] = self[i];
    }
  }
  return arr;
}

// returns the found value in the array, if an element in the array satisfies the provided testing function or undefined if not found
find(callback) {
  assertEx(isDefined(self), "caller undefined, should be array");
  assertEx(isArray(self), "caller should be an array");
  assertEx(isDefined(callback), "argument is undefined");

  for (i = 0; i < self.size; i++) {
    if ([[callback]](self[i], i, self)) {
      return self[i];
    }
  }
  return undefined;
}

// returns the found index in the array, if an element in the array satisfies the provided testing function or -1 if not found
findIndex(callback) {
  assertEx(isDefined(self), "caller undefined, should be array");
  assertEx(isArray(self), "caller should be an array");
  assertEx(isDefined(callback), "argument is undefined");

  for (i = 0; i < self.size; i++) {
    if ([[callback]](self[i], i, self)) {
      return i;
    }
  }
  return -1;
}

// calls a function for each element in the array
forEach(callback) {
  assertEx(isDefined(self), "caller undefined, should be array");
  assertEx(isArray(self), "caller should be an array");
  assertEx(isDefined(callback), "argument is undefined");

  for (i = 0; i < self.size; i++) {
    [[callback]](self[i], i, self);
  }
}

// get length of an array
getLength() {
  assertEx(isDefined(self), "caller undefined, should be array");
  assertEx(isArray(self), "caller should be an array");

  return self.size;
}

// determines whether an array contains a certain element, returning true or false as appropriate
includes(item) {
  assertEx(isDefined(self), "caller undefined, should be array");
  assertEx(isArray(self), "caller should be an array");
  assertEx(isDefined(item), "argument is undefined");

  for (i = 0; i < self.size; i++) {
    if (self[i] == item) {
      return true;
    }
  }
  return false;
}

// returns the first index of an element within the array equal to the specified value, or -1 if none is found
indexOf(item) {
  assertEx(isDefined(self), "caller undefined, should be array");
  assertEx(isArray(self), "caller should be an array");
  assertEx(isDefined(item), "argument is undefined");

  for (i = 0; i < self.size; i++) {
    if (self[i] == item) {
      return i;
    }
  }
  return -1;
}

// joins all elements of an array into a string
join() {
  assertEx(isDefined(self), "caller undefined, should be array");
  assertEx(isArray(self), "caller should be an array");

  str = "";
  for (i = 0; i < self.size; i++) {
    str += self[i];
  }
  return str;
}

// returns the last index of an element within the array equal to the specified value, or -1 if none is found
lastIndexOf(item) {
  assertEx(isDefined(self), "caller undefined, should be array");
  assertEx(isArray(self), "caller should be an array");
  assertEx(isDefined(item), "argument is undefined");

  for (i = self.size - 1; i >= 0; i--) {
    if (self[i] == item) {
      return i;
    }
  }
  return -1;
}

// creates a new array with the results of calling a provided function on every element in this array
mapArray(callback) {
  assertEx(isDefined(self), "caller undefined, should be array");
  assertEx(isArray(self), "caller should be an array");
  assertEx(isDefined(callback), "argument is undefined");

  arr = [];
  for (i = 0; i < self.size; i++) {
    arr[arr.size] = [[callback]](self[i], i, self);
  }
  return arr;
}

// removes the last element from an array
pop() {
  assertEx(isDefined(self), "caller undefined, should be array");
  assertEx(isArray(self), "caller should be an array");

  arr = [];
  for (i = 0; i < self.size - 1; i++) {
    arr[i] = self[i];
  }
  return arr;
}

// adds one or more elements to the end of an array
push(item) {
  assertEx(isDefined(self), "caller undefined, should be array");
  assertEx(isArray(self), "caller should be an array");
  assertEx(isDefined(item), "argument is undefined");

  arr = self;
  if (isArray(item)) {
    for (i = 0; i < item.size; i++) {
      arr[arr.size] = item[i];
    }
  } else {
    arr[arr.size] = item;
  }
  return arr;
}

// apply a function against an accumulator and each value of the array (from left-to-right) as to reduce it to a single value
reduce(callback, initialValue) {
  assertEx(isDefined(self), "caller undefined, should be array");
  assertEx(isArray(self), "caller should be an array");
  assertEx(isDefined(callback), "argument is undefined");

  acc = initialValue;
  for (i = 0; i < self.size; i++) {
    acc = [[callback]](acc, self[i], i, self);
  }
  return acc;
}

// apply a function against an accumulator and each value of the array (from right-to-left) as to reduce it to a single value
reduceRight(callback, initialValue) {
  assertEx(isDefined(self), "caller undefined, should be array");
  assertEx(isArray(self), "caller should be an array");
  assertEx(isDefined(callback), "argument is undefined");

  acc = initialValue;
  for (i = self.size - 1; i >= 0; i--) {
    acc = [[callback]](acc, self[i], i, self);
  }
  return acc;
}

// reverses the order of the elements of an array in place
// the first becomes the last, and the last becomes the first
reverse() {
  assertEx(isDefined(self), "caller undefined, should be array");
  assertEx(isArray(self), "caller should be an array");

  arr = [];
  for (i = 0; i < self.size; i++) {
    arr[self.size - i - 1] = self[i];
  }
  return arr;
}

// removes the first element from an array
shift() {
  assertEx(isDefined(self), "caller undefined, should be array");
  assertEx(isArray(self), "caller should be an array");

  arr = [];
  for (i = 0; i < self.size - 1; i++) {
    arr[i] = self[i + 1];
  }
  return arr;
}

// extracts a section of an array
slice(start, end) {
  assertEx(isDefined(self), "caller undefined, should be array");
  assertEx(isArray(self), "caller should be an array");
  assertEx(isDefined(start), "argument is undefined");

  if (start < 0) {
    start = self.size + start;
  }
  if (!isDefined(end)) {
    end = self.size;
  } else if (end < 0) {
    end = self.size + end;
  }

  arr = [];
  for (i = start; i < end; i++) {
    arr[arr.size] = self[i];
  }
  return arr;
}

// returns true if at least one element in this array satisfies the provided testing function
some(callback) {
  assertEx(isDefined(self), "caller undefined, should be array");
  assertEx(isArray(self), "caller should be an array");
  assertEx(isDefined(callback), "argument is undefined");

  for (i = 0; i < self.size; i++) {
    if ([[callback]](self[i], i, self)) {
      return true;
    }
  }
  return false;
}

// adds and/or removes elements from an array
splice(start, deleteCount, items) {
  assertEx(isDefined(self), "caller undefined, should be array");
  assertEx(isArray(self), "caller should be an array");
  assertEx(isDefined(start), "argument is undefined");

  if (!isDefined(deleteCount)) {
    deleteCount = self.size - start;
  }

  if (start < 0) {
    start = self.size + start;
  }

  arr = [];
  for (i = 0; i < self.size; i++) {
    if (i >= start && i < start + deleteCount) {
      continue;
    }
    arr[arr.size] = self[i];
  }
  result = arr;

  if (isDefined(items)) {
    if (!isArray(items)) {
      arr = [];
      arr[0] = items;
      items = arr;
    }

    arr = [];
    for (i = 0; i < result.size; i++) {
      if (i == start) {
        for (j = 0; j < items.size; j++) {
          arr[arr.size] = items[j];
        }
      }
      arr[arr.size] = result[i];
    }
    result = arr;
  }

  return result;
}

// adds one or more elements to the front of an array
unshift(item) {
  assertEx(isDefined(self), "caller undefined, should be array");
  assertEx(isArray(self), "caller should be an array");
  assertEx(isDefined(item), "argument is undefined");

  if (!isArray(item)) {
    items = [];
    items[0] = item;
    item = items;
  }

  arr = [];
  for (i = 0; i < item.size; i++) {
    arr[arr.size] = item[i];
  }
  for (i = 0; i < self.size; i++) {
    arr[arr.size] = self[i];
  }
  return arr;
}
