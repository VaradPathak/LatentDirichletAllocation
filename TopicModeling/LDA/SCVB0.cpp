/*
 * SCVB0.cpp
 *
 *  Created on: Mar 7, 2014
 *      Author: Varad Pathak, Arul Samuel
 */

#include "SCVB0.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <algorithm>
#include "MiniBatch.h"

using namespace std;

SCVB0::SCVB0(int iter, int numberOfTopics, int vocabSize, int numOfDocs,
		int corpusSize) {
	iterations = iter;
	K = numberOfTopics;
	W = vocabSize;
	D = numOfDocs;
	C = corpusSize;
	numOfBurnInPasses = 5;

	s = 1;
	tau = 10;
	kappa = 0.9;


	rhoPhi = s / pow((tau + 1), kappa);
	rhoTheta = s / pow((tau + 1), kappa);

	alpha = 0.1;
	eta = 0.01;

	nPhi = new double*[W + 1];
	nTheta = new double*[D + 1];
	nz = new double[K];
	memset(nz, 0, sizeof(nz));
	srand(time(NULL));
	for (int w = 0; w < W + 1; w++) {
		nPhi[w] = new double[K];
		for (int k = 0; k < K; ++k) {
			nPhi[w][k] = ((double) (rand() % (W * K))) / (W * K);
			nz[k] += nPhi[w][k];
		}
	}
	for (int d = 0; d < D + 1; d++) {
		nTheta[d] = new double[K];
		for (int k = 0; k < K; ++k) {
			nTheta[d][k] = ((double) (rand() % (D * K))) / (D * K);
		}
	}
}

SCVB0::~SCVB0() {
	for (int w = 0; w < W + 1; w++) {
		delete[] (nPhi[w]);
	}
	for (int d = 0; d < D + 1; d++) {
		delete[] (nTheta[d]);
	}
	delete[] (nPhi);
	delete[] (nTheta);
	delete[] (nz);
}

void SCVB0::run(MiniBatch miniBatch) {

	double **nPhiHat = new double*[W + 1];
	double *nzHat = new double[K];
	double **gamma = new double*[W + 1];

	for (int w = 0; w < W + 1; w++) {
		nPhiHat[w] = new double[K];
		gamma[w] = new double[K];
		memset(nPhiHat[w], 0, sizeof(double)*K);
		memset(gamma[w], 0, sizeof(double)*K);
	}
	memset(nzHat, 0, sizeof(double)*K);

	// This is where original run method starts
	vector<Document> *docVector = miniBatch.docVector;
	random_shuffle(docVector->begin(), docVector->end());
	cout << "MiniBatchSize: " << miniBatch.M << endl;
	for (std::vector<Document>::iterator it = docVector->begin(); it != docVector->end(); it++) {
		Document doc = *it;
		for (int counter = 1; counter <= numOfBurnInPasses; counter++) {
			rhoTheta = s / pow((tau + counter), kappa);
			for (map<int, int>::iterator iter = doc.termDict.begin(); iter != doc.termDict.end(); iter++) {
				int term = iter->first;
				int k = 0;
				for (k = 0; k < K; k++) {
					gamma[term][k] = ((nPhi[term][k] + eta) * (nTheta[doc.docId][k] + alpha) / (nz[k] + eta * miniBatch.M));

					nTheta[doc.docId][k] = ((pow((1 - rhoTheta), doc.termDict[term]) * nTheta[doc.docId][k])
							+ ((1 - pow((1 - rhoTheta), doc.termDict[term])) * doc.Cj * gamma[term][k]));
				}
			}
		}
		int t = 0;
		for (map<int, int>::iterator iter = doc.termDict.begin(); iter != doc.termDict.end(); ++iter) {
			int term = iter->first;
			t++;
			rhoTheta = s / pow((tau + t + numOfBurnInPasses), kappa);

			for (int k = 0; k < K; k++) {
				gamma[term][k] = ((nPhi[term][k] + eta) * (nTheta[doc.docId][k] + alpha)/ (nz[k] + eta * miniBatch.M));
				nTheta[doc.docId][k] = ((pow((1 - rhoTheta), doc.termDict[term]) * nTheta[doc.docId][k])
						+ ((1 - pow((1 - rhoTheta), doc.termDict[term])) * doc.Cj * gamma[term][k]));

				nPhiHat[term][k] = nPhiHat[term][k] + (C * gamma[term][k]/ miniBatch.M);
				nzHat[k] = nzHat[k] + (C * gamma[term][k]/ miniBatch.M);
			}
		}
	}

	for (int k = 0; k < K; k++) {
		for (int w = 1; w < W + 1; w++) {
			rhoPhi = s / pow((tau + (w*k)), kappa);
			nPhi[w][k] = ((1 - rhoPhi) * nPhi[w][k]) + (rhoPhi * nPhiHat[w][k]);
		}
		nz[k] = ((1 - rhoPhi) * nz[k]) + (rhoPhi * nzHat[k]);
	}
	for (int w = 0; w < W + 1; w++) {
		delete[] (gamma[w]);
		delete[] (nPhiHat[w]);
	}
	delete[] (nPhiHat);
	delete[] (gamma);
	delete[] (nzHat);
}
