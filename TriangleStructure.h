#ifndef TRIANGLESTRUCTURE
#define TRIANGLESTRUCTURE


/*
 * A triangle is defined such that it has sides and vertices numbered 0-2,
 * starting at its "top" and going clockwise from there.
 *
 *          0
 *          ^
 *         / \
 *        / * \
 *     2 /     \ 1
 *      / *   * \
 *     /_________\
 *    2     1     1
 *
 * A triangle may have a single neighbor along each edge, but multiple
 * neighbors at each vertex.  If another triangle is a neighbor on an edge
 * then it cannot also neighbor at a vertex.
 */

class Triangle {
 public:

  Triangle *getEdge(byte edge);
  void setEdge(byte edge, Triangle *tri);

  Triangle *getVertex(byte vertex);
  void setVertex(byte vertex, Triangle *tri);

 private:

  Triangle *edges[3];
  Triangle *vertices[3];
};

#endif
