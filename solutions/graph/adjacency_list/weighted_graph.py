class Vertex:
    def __init__(self, label):
        self.label = label
        self.adjacent = {}

    def __str__(self):
        return str(self.label) + ' adjacent: ' + str([x for x in self.adjacent])

    def __repr__(self):
        return self.label

    def add_neighbour(self, neighbour, weight=0):
        self.adjacent[neighbour] = weight

    def get_connections(self):
        return self.adjacent.keys()

    def get_label(self):
        return self.label

    def get_weight(self, neighbour):
        return self.adjacent.get(neighbour)


class WeightedGraph:
    def __init__(self):
        self.vertices = {}

    def __iter__(self):
        return iter(self.vertices.values())

    def add_vertex(self, label):
        if label not in self.vertices:
            vertex = Vertex(label)
            self.vertices[label] = vertex
            return vertex

    def get_vertex(self, label):
        return self.vertices.get(label)

    def add_edge(self, label1, label2, weight):
        if label1 not in self.vertices:
            vertex1 = self.add_vertex(label1)
        else:
            vertex1 = self.get_vertex(label1)
        if label2 not in self.vertices:
            vertex2 = self.add_vertex(label2)
        else:
            vertex2 = self.get_vertex(label2)
        vertex1.add_neighbour(vertex2, weight)
        vertex2.add_neighbour(vertex1, weight)

    def get_vertex_labels(self):
        return self.vertices.keys()


def main():

    g = WeightedGraph()

    g.add_vertex('a')
    g.add_vertex('b')

    g.add_edge('a', 'b', 2)
    g.add_edge('b', 'c', 5)
    g.add_edge('c', 'd', 7)

    for v in g:
        print(v)

if __name__ == '__main__':
    main()
