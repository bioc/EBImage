\name{tile}

\alias{tile}
\alias{untile}

\title{Tiling/untiling images}

\description{
  Given a sequence of frames, \code{tile} generates a single image with frames tiled.
  \code{untile} is the inverse function and divides an image into a sequence of images.
}

\usage{
tile(x, nx=10, lwd=1, fg.col="#E4AF2B", bg.col="gray")
untile(x, nim, lwd=1)
}

\arguments{
  \item{x}{An \code{Image} object, an array or a list of these objects.} 

  \item{nx}{ The number of tiled images in a row. }

  \item{lwd}{ The width of the grid lines between tiled images, can be 0. }

  \item{fg.col}{The color of the grid lines.}

  \item{bg.col}{The color of the background for extra tiles.}

  \item{nim}{A numeric vector of 2 elements for the number of images in both directions.}
}

\value{
 An \code{Image} object or an array, containing the tiled/untiled version
 of \code{x}.
}

\details{
  After object segmentation, \code{tile} is a useful addition to \code{stackObjects}
  to have an overview of the segmented objects.
}

\seealso{ \code{\link{stackObjects}}
}

\examples{
  ## make a set of blurred images
  img = readImage(system.file("images", "sample-color.png", package="EBImage"))[257:768,,]
  x = resize(img, 128, 128)
  xt = list()
  for (t in seq(0.1, 5, length.out=9)) xt=c(xt, list(gblur(x, s=t)))
  xt = combine(xt)
  display(xt, title='Blurred images')

  ## tile
  xt = tile(xt, 3)
  display(xt, title='Tiles')

  ## untile
  xu = untile(img, c(3, 3))
  display(xu, title='Blocks')
}

\author{
  Oleg Sklyar, \email{osklyar@ebi.ac.uk}, 2006-2007
}

\keyword{manip}
