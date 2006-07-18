updateEBImageDev <- function() {
    install.packages("EBImage", repos="http://www.bioconductor.org/packages/1.9/bioc")
    cat("\n\nPlease re-load the library!\n\n")
}

updateEBImage <- function() {
    install.packages("EBImage", repos="http://www.ebi.ac.uk/huber-srv/data/Rrepos")
    cat("\n\nPlease re-load the library!\n\n")
}

# Package initialization routine
.onLoad <- function(lib, pkg) {
    ## use useDynLib("EBImage") in NAMESPACE instead
    ## library.dynam("EBImage", pkg, lib)
    ## cat("* EBImage of Bioconductor.org: help(EBImage) to get started...\n")
    require("methods")

cat("\nWARNING: EBImage API has changed in the development release 1.3.x\n\nIt is strongly recommended to update EBImage\n\nCall 'updateEBImageDev()' to upgrade to the latest development version\nor download it from http://www.bioconductor.org/packages/1.9/bioc/\n\nCall 'updateEBImage()' to upgrade to the latest STABLE version\nor download it from http://www.ebi.ac.uk/~osklyar/projects/EBImage/\n\n")

cat("Do you want to upgrade to the stable version now [yes/no], default = 'no'? ")
up <- readline()
if (tolower(up) == "yes")
    updateEBImage()
}

# shortcut to call library functions for EBImage
.CallEBImage <- function(name, ...) {
    .Call(name, ..., PACKAGE = "EBImage")
}

verboseEBImage <- function(verbose = TRUE) {
    .CallEBImage("setVerbose", as.logical(verbose))
    invisible(FALSE)
}
