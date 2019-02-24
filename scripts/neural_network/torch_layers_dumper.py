import numpy as np


def DumpReLULayer(fh, layer):
	fh.write( "ReLU\n" )


def DumpSigmoidLayer(fh, layer):
	fh.write( "Sigmoid\n" )
	
	
def DumpLinearLayer(fh, layer):
	has_bias = (type(layer.bias) != type(None))

	weight = layer.weight.cpu().data.numpy()
	if has_bias:
		bias = layer.bias.cpu().data.numpy()

	weight = weight.reshape(-1)

	fh.write( "Linear\n" )
	fh.write( str(layer.in_features) + "\n" )
	fh.write( str(layer.out_features) + "\n" )
	if has_bias:
		fh.write("bias_yes\n")
	else:
		fh.write("bias_no\n")
	output = weight
	if has_bias:
		output = np.append(output, bias)
	for i in range(len(output)):
		fh.write( str(output[i]) + "\n" )


def DumpConv2DLayer(fh, layer):
	weight = layer.weight.cpu().data.numpy()
	bias = layer.bias.cpu().data.numpy()
	
	weight = weight.reshape(-1)
	bias = bias.reshape(-1)
	
	fh.write( "Conv2D\n" )
	fh.write( str(layer.in_channels) + "\n" )
	fh.write( str(layer.out_channels) + "\n" )
	fh.write( str(layer.kernel_size[0]) + "\n" )
	fh.write( str(layer.kernel_size[1]) + "\n" )
	output = weight
	output = np.append(output, bias)
	for i in range(len(output)):
		fh.write( str(output[i]) + "\n" )


def DumpBatchNorm2DLayer(fh, layer):
	weight = layer.weight.cpu().data.numpy()
	bias = layer.bias.cpu().data.numpy()
	running_mean = layer.running_mean.cpu().numpy()
	running_var = layer.running_var.cpu().numpy()
	
	fh.write( "BatchNorm2D\n" )
	fh.write( str(layer.num_features) + "\n" )
	output = weight
	output = np.append(output, bias)
	output = np.append(output, running_mean)
	output = np.append(output, running_var)
	for i in range(len(output)):
		fh.write( str(output[i]) + "\n" )


def DumpLayer(fh, layer):
	type_name = type(layer).__name__

	if type_name == "ReLU":
		DumpReLULayer(fh, layer)
	elif type_name == "Sigmoid":
		DumpSigmoidLayer(fh, layer)
	elif type_name == "Linear":
		DumpLinearLayer(fh, layer)
	elif type_name == "Conv2d":
		DumpConv2DLayer(fh, layer)
	elif type_name == "BatchNorm2d":
		DumpBatchNorm2DLayer(fh, layer)


def DumpSequential(path, sequential):
	fh = open(path, "w")
	
	fh.write( str(len(sequential._modules)) + "\n" )

	for key, value in sequential._modules.items():
		DumpLayer(fh, value)
