/* COPYRIGHT (C) 2001 Philippe Elie under GPL license */

#include <fstream>
#include <vector>
#include <map>

#include "../../util/persistent.h"

struct test {
	int a;
	int b;
	std::string c;

	static void describe();
};

std::ostream& operator<<(std::ostream & out, const test & t)
{
	out << t.a << " " << t.b << " " << t.c << " " << std::endl;

	return out;
}

void test::describe()
{
	Persistent<test>::addField(&test::a, "a");
	Persistent<test>::addField(&test::b, "b");
	Persistent<test>::addField(&test::c, "c");
}

int main()
{
	typedef std::vector<test> c_t;
	typedef std::map<std::string, test> m_t;

	test t1 = { 3, 4, "string 1" };
	test t2 = { 1, 2, "str \\\" 2" };

	std::ofstream out("test.pers");
	Persistent<test>::save(t1, out);

	c_t v;
	v.push_back(t1); v.push_back(t2); v.push_back(t1);
	Persistent<test>::saveContainer(v, out);

	m_t m;
	m.insert(m_t::value_type("str 1", t1));
	m.insert(m_t::value_type("str 2", t2));
	Persistent<test>::saveContainer(m, out);

	out.flush();

	std::ifstream in("test.pers");
	Persistent<test>::load(t2, in);

	std::cout << t2;

	v.clear();
	Persistent<test>::loadContainer(v, in);

	for (c_t::const_iterator it = v.begin() ; it != v.end() ; ++it)
		std::cout << *it;

	m.clear();
	Persistent<test>::loadContainer(m, in);
	for (m_t::const_iterator it = m.begin() ; it != m.end() ; ++it)
		std::cout << it->first << " " << it->second;

	return 0;
}
