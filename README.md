# knn_search
k nearest neighbor search algorithm for spatial databases.

## Specifications
**Space Definition**
Dataset is prepared with attributes ([latitude], [longitude], [location id]), which contains spatial
points called [location id], with 2D coordinates ([latitude], [longitude]).

Data set is procured from [here](http://snap.stanford.edu/data/loc-gowalla.html).

The minimum and maximum longitude and latitude values are as follows:
*x_min=-90.0, x_max=90.0, y_min=-176.3, y_max=177.5*

The space defined by maxBox into n * n equal-sized rectangles called cells.

## Workflow
**Data Deduplication**

First, the data points with the same locations but with different location ids are deduplicated, retaining
the data point with lower location id value.

**Allocation of data points inside the space**

For each data point p, assign p to the corresponding cell such that p’s coordinates are located within it.
Ff p is located on boundaries but not on maxBox, then it should belong to the cell on its left or its top. 

**k-nearest neighbors search**

Find the cell e that contains q. For each point p in e (if any), find the k-nearest neighbors of q by
computing the Euclidean distance of q and p. Keep track of the k-NN set, and let t be the k-th largest
distance in the k-NN set.

b. Progressively access the cells layered around e1, and for each such cell c,

i) compute dlow(c), which is the smallest distance from q;

ii) if dlow(c)>t, skip the cell (explain why is this possible in the report);

iii) otherwise access all points in c, compute their distances to q, and update k-NN.


c. Once the algorithm prunes an entire layer of cells, or finishes examining all the cells, report the query
results.
