#include <globjects/Query.h>

#include <glbinding/gl/functions.h>
#include <glbinding/gl/boolean.h>
#include <glbinding/gl/enum.h>

#include <globjects/ObjectVisitor.h>

#include "Resource.h"

namespace globjects
{

Query::Query()
: Object(new QueryResource)
{
}

Query::Query(IDResource * resource)
: Object(resource)
{
}

Query * Query::fromId(gl::GLuint id)
{
    return new Query(new ExternalResource(id));
}

Query::~Query()
{
}

Query * Query::current(gl::GLenum target)
{
    gl::GLint id = get(target, gl::GL_CURRENT_QUERY);

    if (id <= 0)
    {
        return nullptr;
    }
	
    // TODO: fetch correct query from object registry
    return Query::fromId(id);
}

Query * Query::timestamp()
{
    Query * query = new Query();
    query->counter(gl::GL_TIMESTAMP);

    return query;
}

int Query::getCounterBits(gl::GLenum target)
{	
    return get(target, gl::GL_QUERY_COUNTER_BITS);
}

gl::GLuint Query::genQuery()
{
	gl::GLuint id;

    gl::glGenQueries(1, &id);

	return id;
}

gl::GLint Query::get(gl::GLenum target, gl::GLenum pname)
{
    gl::GLint value = 0;

    gl::glGetQueryiv(target, pname, &value);

    return value;
}

gl::GLint Query::getIndexed(gl::GLenum target, gl::GLuint index, gl::GLenum pname)
{
    gl::GLint value = 0;

    gl::glGetQueryIndexediv(target, index, pname, &value);

    return value;
}

void Query::accept(ObjectVisitor& visitor)
{
	visitor.visitQuery(this);
}

void Query::begin(gl::GLenum target) const
{
    gl::glBeginQuery(target, id());
}

void Query::end(gl::GLenum target) const
{
    gl::glEndQuery(target);
}

void Query::beginIndexed(gl::GLenum target, gl::GLuint index) const
{
    gl::glBeginQueryIndexed(target, index, id());
}

void Query::endIndexed(gl::GLenum target, gl::GLuint index) const
{
    gl::glEndQueryIndexed(target, index);
}

gl::GLuint Query::get(gl::GLenum pname) const
{
    gl::GLuint value = 0;
	
    gl::glGetQueryObjectuiv(id(), pname, &value);
	
	return value;
}

gl::GLuint64 Query::get64(gl::GLenum pname) const
{
    gl::GLuint64 value = 0;
	
    gl::glGetQueryObjectui64v(id(), pname, &value);
	
	return value;
}

bool Query::resultAvailable() const
{
    return static_cast<gl::GLboolean>(get(gl::GL_QUERY_RESULT_AVAILABLE)) == gl::GL_TRUE;
}

void Query::wait() const
{
    while (!resultAvailable());
}

void Query::wait(const std::chrono::duration<int, std::nano> & timeout) const
{
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    std::chrono::high_resolution_clock::time_point current;
    while (!resultAvailable() && start + timeout > current)
    {
        current = std::chrono::high_resolution_clock::now();
    }
}

gl::GLuint Query::waitAndGet(gl::GLenum pname) const
{
    wait();
	
	return get(pname);
}

gl::GLuint64 Query::waitAndGet64(gl::GLenum pname) const
{
    wait();
	
    return get64(pname);
}

gl::GLuint Query::waitAndGet(const std::chrono::duration<int, std::nano> & timeout, gl::GLenum pname) const
{
    return waitAndGet(pname, timeout);
}

gl::GLuint64 Query::waitAndGet64(const std::chrono::duration<int, std::nano> & timeout, gl::GLenum pname) const
{
    return waitAndGet64(pname, timeout);
}

gl::GLuint Query::waitAndGet(gl::GLenum pname, const std::chrono::duration<int, std::nano> & timeout) const
{
    wait(timeout);

    return get(pname);
}

gl::GLuint64 Query::waitAndGet64(gl::GLenum pname, const std::chrono::duration<int, std::nano> & timeout) const
{
    wait(timeout);

    return get64(pname);
}

void Query::counter() const
{
    counter(gl::GL_TIMESTAMP);
}

void Query::counter(gl::GLenum target) const
{
    gl::glQueryCounter(id(), target);
}

bool Query::isQuery(gl::GLuint id)
{
    return gl::glIsQuery(id) == gl::GL_TRUE;
}

gl::GLenum Query::objectType() const
{
    return gl::GL_QUERY;
}

} // namespace globjects
