# TODO parametrize paths
data = as.matrix(read.table("/tmp/gnuplotter.dat"))
library(fastICA)
ica = fastICA(data, 2)
centers <- attributes(ica$X)$`scaled:center`
# X <- t(apply(data,1,"-",centers))
# S <- X %*% ica$K %*% ica$W
KW <- ica$K %*% ica$W
write(centers,file="config/centers.dat",ncolumns=14)
write(t(ica$K),file="config/K.dat",ncolumns=28)
write(t(ica$W),file="config/W.dat",ncolumns=4)
write(t(KW),file="config/KW.dat",ncolumns=28)

pdf("log/ica.pdf")
plot(seq(length(ica$S[,1])), ica$S[,1])
plot(seq(length(ica$S[,2])), ica$S[,2])
dev.off()
