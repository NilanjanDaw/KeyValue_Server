/**
 * @Author: nilanjan
 * @Date:   2018-10-03T00:05:47+05:30
 * @Email:  nilanjandaw@gmail.com
 * @Filename: server_single.c
 * @Last modified by:   nilanjan
 * @Last modified time: 2018-10-03T00:29:08+05:30
 * @Copyright: Nilanjan Daw
 */
#include <unordered_map>

using namespace std;

int main(int argc, char const *argv[]) {
  unordered_map<int, char*> hashtable;
  char *buffer = "abcd";
  hashtable[1234] = buffer;
  hashtable[34] = "dfssfd";
  printf("%s\n", hashtable[1234]);
  printf("%s\n", hashtable[14]);
  return 0;
}
