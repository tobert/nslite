Name:             nslite
Version:          0.0.1
Release:          1%{?dist}
Summary:          Lightweight Linux namespace utilities

Group:            Applications/Virtualization
License:          Artistic 2
URL:              https://github.com/tobert/nslite
# create with:
# git clone git@github.com:tobert/nslite.git
# git archive --format=tar --prefix=nslite-0.0.1/ 2d4118e0afc57a2c4d82ddda2fb058a0724e8c3e |gzip -c > ~/rpmbuild/SOURCES/nslite-0.0.1.tar.gz
Source0:          %{name}-%{version}.tar.gz

%description
NSLite 0.0.1 contains nschroot, a chroot utility that also creates Linux
namespaces for the child process.

%prep

%setup -q

%build

make nschroot

%install
install -p -D -m 755 nschroot %{buildroot}%{_bindir}/nschroot

%files
%defattr(-,root,root,-)
%{_bindir}/nschroot


%changelog
* Tue Mar 08 2011 Al Tobey <tobert@gmail.com> - 0.0.1-1
- create spec

