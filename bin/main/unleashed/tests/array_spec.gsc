#include unleashed\array;
#include unleashed\spec;

main() {
  describe("GSC module: array");

  spec = describe("-> create");
  it(spec, "should return an empty array", ::spec_array_create);

  spec = describe("-> getLength");
  it(spec, "should return the correct length", ::spec_array_getLength);

  spec = describe("-> includes");
  it(spec, "should return false for an empty array", ::spec_array_includes_empty);
  it(spec, "should return false if the item is not present", ::spec_array_includes_false);
  it(spec, "should return true if the item is present", ::spec_array_includes_true);

  spec = describe("-> indexOf");
  it(spec, "should return -1 for an empty array", ::spec_array_indexOf_empty);
  it(spec, "should return -1 if the item is not present", ::spec_array_indexOf_notFound);
  it(spec, "should return the correct index if the item is present", ::spec_array_indexOf_found);
  it(spec, "should return the correct index if the item is present multiple times", ::spec_array_indexOf_foundMultiple);

  spec = describe("-> lastIndexOf");
  it(spec, "should return -1 for an empty array", ::spec_array_lastIndexOf_empty);
  it(spec, "should return -1 if the item is not present", ::spec_array_lastIndexOf_notFound);
  it(spec, "should return the correct index if the item is present", ::spec_array_lastIndexOf_found);
  it(spec, "should return the correct index if the item is present multiple times", ::spec_array_lastIndexOf_foundMultiple);

  spec = describe("-> every");
  it(spec, "should return true for an empty array", ::spec_array_every_empty);
  it(spec, "should return false if not all items are true", ::spec_array_every_false);
  it(spec, "should return true if all items are true", ::spec_array_every_true);

  spec = describe("-> some");
  it(spec, "should return false for an empty array", ::spec_array_some_empty);
  it(spec, "should return false if all items are false", ::spec_array_some_false);
  it(spec, "should return true if not all items are false", ::spec_array_some_true);

  spec = describe("-> filter");
  it(spec, "should return a correctly filtered array", ::spec_array_filter);

  spec = describe("-> find");
  it(spec, "should return the correct value", ::spec_array_find);

  spec = describe("-> findIndex");
  it(spec, "should return the correct index", ::spec_array_findIndex);

  spec = describe("-> forEach");
  it(spec, "should call the function for each element in the array", ::spec_array_forEach);

  spec = describe("-> join");
  it(spec, "should return the correct string value", ::spec_array_join);

  spec = describe("-> map");
  it(spec, "should call the function for each element in the array and return the correct mapped values", ::spec_array_mapArray);

  spec = describe("-> pop");
  it(spec, "should remove the last element from the array", ::spec_array_pop);

  spec = describe("-> push");
  it(spec, "should append an item to the end of the array", ::spec_array_push);
  it(spec, "should append all items to the end of the array", ::spec_array_pushMultiple);

  spec = describe("-> unshift");
  it(spec, "should prepend an item to the start of the array", ::spec_array_unshift);
  it(spec, "should prepend all items to the start of the array", ::spec_array_unshiftMultiple);

  spec = describe("-> reverse");
  it(spec, "should reverse the array", ::spec_array_reverse);

  spec = describe("-> shift");
  it(spec, "should remove the first element from the array", ::spec_array_shift);

  spec = describe("-> slice");
  it(spec, "should return the elements between the start and end indices", ::spec_array_slice_start_end);
  it(spec, "should return all elements from the start index if no end index is provided", ::spec_array_slice_start_only);
  it(spec, "should return the elements between the start (offset from the end) and end indices", ::spec_array_slice_negative_start_positive_end);
  it(spec, "should return the elements between the start (offset from the end) and end (offset from the end) indices", ::spec_array_slice_negative_start_negative_end);
  it(spec, "should return all elements from the start (offset from the end) if no end index is provided", ::spec_array_slice_negative_start_only);

  spec = describe("-> splice");
  it(spec, "should delete the elements between the start and start + delete count indices", ::spec_array_splice_start_deleteCount);
  it(spec, "should delete all elements from the start index", ::spec_array_splice_start_only);
  it(spec, "should delete the elements between the start and start + delete count indices and insert the new items", ::spec_array_splice_newItems);

  spec = describe("-> reduce");
  it(spec, "should call the function on each item and return the reduced result", ::spec_array_reduce);

  spec = describe("-> reduceRight");
  it(spec, "should call the function on each item from right-to-left and return the reduced result", ::spec_array_reduceRight);
}

spec_array_create() {
  result = create();
  assertThat(isArray(result), "result should be an array");
  assertThat(result.size == 0, "array should be empty");
}

spec_array_getLength() {
  arr = [];
  assertThat(arr getLength() == 0, "array should be empty");

  arr[arr.size] = "a";
  arr[arr.size] = "b";
  assertThat(arr getLength() == 2, "array should contain 2 items");
}

spec_array_includes_empty() {
  arr = [];
  assertThat(arr includes(1) == false, "should return false");
}

spec_array_includes_false() {
  arr = [];
  arr[arr.size] = 5;
  arr[arr.size] = 8;
  assertThat(arr includes(1) == false, "should return false");
}

spec_array_includes_true() {
  arr = [];
  arr[arr.size] = 5;
  arr[arr.size] = 8;
  assertThat(arr includes(5) == true, "should return true");
}

spec_array_indexOf_empty() {
  arr = [];
  assertThat(arr indexOf(5) == -1, "should return -1");
}

spec_array_indexOf_notFound() {
  arr = [];
  arr[arr.size] = 5;
  arr[arr.size] = 8;
  assertThat(arr indexOf(2) == -1, "should return -1");
}

spec_array_indexOf_found() {
  arr = [];
  arr[arr.size] = 5;
  arr[arr.size] = 8;
  assertThat(arr indexOf(8) == 1, "should return 1");
}

spec_array_indexOf_foundMultiple() {
  arr = [];
  arr[arr.size] = 5;
  arr[arr.size] = 8;
  arr[arr.size] = 5;
  arr[arr.size] = 8;
  assertThat(arr indexOf(8) == 1, "should return 1");
}

spec_array_lastIndexOf_empty() {
  arr = [];
  assertThat(arr lastIndexOf(5) == -1, "should return -1");
}

spec_array_lastIndexOf_notFound() {
  arr = [];
  arr[arr.size] = 5;
  arr[arr.size] = 8;
  assertThat(arr lastIndexOf(2) == -1, "should return -1");
}

spec_array_lastIndexOf_found() {
  arr = [];
  arr[arr.size] = 5;
  arr[arr.size] = 8;
  assertThat(arr lastIndexOf(8) == 1, "should return 1");
}

spec_array_lastIndexOf_foundMultiple() {
  arr = [];
  arr[arr.size] = 5;
  arr[arr.size] = 8;
  arr[arr.size] = 5;
  arr[arr.size] = 8;
  assertThat(arr lastIndexOf(8) == 3, "should return 3");
}

helper_spec_array_every(item) {
  return item == true;
}

spec_array_every_empty() {
  arr = [];
  assertThat(arr every(::helper_spec_array_every) == true, "should return true");
}

spec_array_every_false() {
  arr = [];
  arr[arr.size] = true;
  arr[arr.size] = false;
  arr[arr.size] = false;
  assertThat(arr every(::helper_spec_array_every) == false, "should return false");
}

spec_array_every_true() {
  arr = [];
  arr[arr.size] = true;
  arr[arr.size] = true;
  arr[arr.size] = true;
  assertThat(arr every(::helper_spec_array_every) == true, "should return true");
}

spec_array_some_empty() {
  arr = [];
  assertThat(arr some(::helper_spec_array_every) == false, "should return false");
}

spec_array_some_false() {
  arr = [];
  arr[arr.size] = false;
  arr[arr.size] = false;
  arr[arr.size] = false;
  assertThat(arr some(::helper_spec_array_every) == false, "should return false");
}

spec_array_some_true() {
  arr = [];
  arr[arr.size] = true;
  arr[arr.size] = false;
  arr[arr.size] = false;
  assertThat(arr some(::helper_spec_array_every) == true, "should return true");
}

helper_spec_array_filter(item) {
  return item == true;
}

spec_array_filter() {
  arr = [];
  arr[arr.size] = true;
  arr[arr.size] = false;
  arr[arr.size] = false;
  arr[arr.size] = true;

  result = arr filter(::helper_spec_array_filter);
  assertThat(result.size == 2, "should have 2 elements");
  assertThat(result[0] == true, "should be true");
  assertThat(result[1] == true, "should be true");
}

helper_spec_array_find(item) {
  return item == "abc";
}

spec_array_find() {
  arr = [];
  arr[arr.size] = "123";
  arr[arr.size] = "abc";
  arr[arr.size] = "789";
  arr[arr.size] = "xyz";

  assertThat(arr find(::helper_spec_array_find) == "abc", "should return abc");
}

spec_array_findIndex() {
  arr = [];
  arr[arr.size] = "123";
  arr[arr.size] = "abc";
  arr[arr.size] = "789";
  arr[arr.size] = "xyz";

  assertThat(arr findIndex(::helper_spec_array_find) == 1, "should return 1");
}

helper_spec_array_forEach(item) {
  if (!isDefined(level.helper_spec_array_forEach_result)) {
    level.helper_spec_array_forEach_result = "";
  }

  level.helper_spec_array_forEach_result += item;
}

spec_array_forEach() {
  arr = [];
  arr[arr.size] = "123";
  arr[arr.size] = "abc";
  arr[arr.size] = "789";
  arr[arr.size] = "xyz";

  arr forEach(::helper_spec_array_forEach);
  assertThat(isDefined(level.helper_spec_array_forEach_result), "should be defined");
  assertThat(level.helper_spec_array_forEach_result == "123abc789xyz", "should return 123abc789xyz");
}

spec_array_join() {
  arr = [];
  arr[arr.size] = "123";
  arr[arr.size] = "abc";
  arr[arr.size] = "789";
  arr[arr.size] = "xyz";
  assertThat(arr join() == "123abc789xyz", "should return 123abc789xyz");
}

helper_spec_array_mapArray(item, index) {
  return "" + index + ": " + item;
}

spec_array_mapArray() {
  arr = [];
  arr[arr.size] = "123";
  arr[arr.size] = "abc";

  result = arr mapArray(::helper_spec_array_mapArray);
  assertThat(result[0] == "0: 123", "should be \"0: 123\"");
  assertThat(result[1] == "1: abc", "should be \"1: abc\"");
}

spec_array_pop() {
  arr = [];
  arr[arr.size] = "123";
  arr[arr.size] = "abc";

  result = arr pop();
  assertThat(result.size == 1, "array should have 1 element");
  assertThat(result[0] == "123", "remaining element should be 123");
}

spec_array_push() {
  arr = [];
  arr[arr.size] = "123";

  result = arr push("abc");
  assertThat(result.size == 2, "array should have 2 elements");
  assertThat(result[0] == "123", "should be 123");
  assertThat(result[1] == "abc", "should be abc");
}

spec_array_pushMultiple() {
  arr = [];
  arr[arr.size] = "123";

  itemsToAdd = [];
  itemsToAdd[itemsToAdd.size] = "abc";
  itemsToAdd[itemsToAdd.size] = "xyz";

  result = arr push(itemsToAdd);
  assertThat(result.size == 3, "array should have 3 elements");
  assertThat(result[0] == "123", "should be 123");
  assertThat(result[1] == "abc", "should be abc");
  assertThat(result[2] == "xyz", "should be xyz");
}

spec_array_unshift() {
  arr = [];
  arr[arr.size] = "123";

  result = arr unshift("abc");
  assertThat(result.size == 2, "array should have 2 elements");
  assertThat(result[0] == "abc", "should be abc");
  assertThat(result[1] == "123", "should be 123");
}

spec_array_unshiftMultiple() {
  arr = [];
  arr[arr.size] = "123";

  itemsToAdd = [];
  itemsToAdd[itemsToAdd.size] = "abc";
  itemsToAdd[itemsToAdd.size] = "xyz";

  result = arr unshift(itemsToAdd);
  assertThat(result.size == 3, "array should have 3 elements");
  assertThat(result[0] == "abc", "should be abc");
  assertThat(result[1] == "xyz", "should be xyz");
  assertThat(result[2] == "123", "should be 123");
}

spec_array_reverse() {
  arr = [];
  arr[arr.size] = "123";
  arr[arr.size] = "abc";
  arr[arr.size] = "789";
  arr[arr.size] = "xyz";

  result = arr reverse();
  assertThat(result[0] == "xyz", "should be xyz");
  assertThat(result[1] == "789", "should be 789");
  assertThat(result[2] == "abc", "should be abc");
  assertThat(result[3] == "123", "should be 123");
}

spec_array_shift() {
  arr = [];
  arr[arr.size] = "123";
  arr[arr.size] = "abc";

  result = arr shift();
  assertThat(result.size == 1, "array should have 1 element");
  assertThat(result[0] == "abc", "remaining element should be 123");
}

spec_array_slice_start_end() {
  arr = [];
  arr[arr.size] = "123";
  arr[arr.size] = "abc";
  arr[arr.size] = "789";
  arr[arr.size] = "xyz";

  result = arr slice(1, 2);
  assertThat(result.size == 1, "array should have 1 element");
  assertThat(result[0] == "abc", "should be abc");

  result = arr slice(1, 3);
  assertThat(result.size == 2, "array should have 2 elements");
  assertThat(result[0] == "abc", "should be abc");
  assertThat(result[1] == "789", "should be 789");
}

spec_array_slice_start_only() {
  arr = [];
  arr[arr.size] = "123";
  arr[arr.size] = "abc";
  arr[arr.size] = "789";
  arr[arr.size] = "xyz";

  result = arr slice(1);
  assertThat(result.size == 3, "array should have 3 elements");
  assertThat(result[0] == "abc", "should be abc");
  assertThat(result[1] == "789", "should be 789");
  assertThat(result[2] == "xyz", "should be xyz");

  result = arr slice(2);
  assertThat(result.size == 2, "array should have 2 elements");
  assertThat(result[0] == "789", "should be 789");
  assertThat(result[1] == "xyz", "should be xyz");
}

spec_array_slice_negative_start_positive_end() {
  arr = [];
  arr[arr.size] = "123";
  arr[arr.size] = "abc";
  arr[arr.size] = "789";
  arr[arr.size] = "xyz";

  result = arr slice(-3, 3);
  assertThat(result.size == 2, "array should have 2 elements");
  assertThat(result[0] == "abc", "should be abc");
  assertThat(result[1] == "789", "should be 789");
}

spec_array_slice_negative_start_negative_end() {
  arr = [];
  arr[arr.size] = "123";
  arr[arr.size] = "abc";
  arr[arr.size] = "789";
  arr[arr.size] = "xyz";

  result = arr slice(-3, -1);
  assertThat(result.size == 2, "array should have 2 elements");
  assertThat(result[0] == "abc", "should be abc");
  assertThat(result[1] == "789", "should be 789");
}

spec_array_slice_negative_start_only() {
  arr = [];
  arr[arr.size] = "123";
  arr[arr.size] = "abc";
  arr[arr.size] = "789";
  arr[arr.size] = "xyz";

  result = arr slice(-2);
  assertThat(result.size == 2, "array should have 2 elements");
  assertThat(result[0] == "789", "should be 789");
  assertThat(result[1] == "xyz", "should be xyz");
}

spec_array_splice_start_deleteCount() {
  arr = [];
  arr[arr.size] = "123";
  arr[arr.size] = "abc";
  arr[arr.size] = "789";
  arr[arr.size] = "xyz";

  result = arr splice(1, 2);
  assertThat(result.size == 2, "array should have 2 elements");
  assertThat(result[0] == "123", "should be 123");
  assertThat(result[1] == "xyz", "should be xyz");

  result = arr splice(0, 3);
  assertThat(result.size == 1, "array should have 1 element");
  assertThat(result[0] == "xyz", "should be xyz");
}

spec_array_splice_start_only() {
  arr = [];
  arr[arr.size] = "123";
  arr[arr.size] = "abc";
  arr[arr.size] = "789";
  arr[arr.size] = "xyz";

  result = arr splice(1);
  assertThat(result.size == 1, "array should have 1 elements");
  assertThat(result[0] == "123", "should be 123");
}

spec_array_splice_newItems() {
  arr = [];
  arr[arr.size] = "123";
  arr[arr.size] = "abc";
  arr[arr.size] = "789";
  arr[arr.size] = "xyz";

  itemsToAdd = [];
  itemsToAdd[itemsToAdd.size] = "456";
  itemsToAdd[itemsToAdd.size] = "def";

  result = arr splice(1, 2, itemsToAdd);
  assertThat(result.size == 4, "array should have 4 elements");
  assertThat(result[0] == "123", "should be 123");
  assertThat(result[1] == "456", "should be 456");
  assertThat(result[2] == "def", "should be def");
  assertThat(result[3] == "xyz", "should be xyz");
}

helper_spec_array_reduce(acc, current) {
  return acc + current;
}

spec_array_reduce() {
  arr = [];
  arr[arr.size] = 1;
  arr[arr.size] = 2;
  arr[arr.size] = 3;
  arr[arr.size] = 4;

  assertThat(arr reduce(::helper_spec_array_reduce, 0) == 10, "should return 10");
}

helper_spec_array_reduceRight(acc, current) {
  return acc + current;
}

spec_array_reduceRight() {
  arr = [];
  arr[arr.size] = 1;
  arr[arr.size] = 2;
  arr[arr.size] = 3;
  arr[arr.size] = 4;

  assertThat(arr reduceRight(::helper_spec_array_reduceRight, "") == "4321", "should return \"4321\"");
}
